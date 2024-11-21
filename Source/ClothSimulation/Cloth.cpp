// Fill out your copyright notice in the Description page of Project Settings.


#include "Cloth.h"
#include "ClothParticle.h"
#include "ClothConstraint.h"
#include "ClothSphere.h"
#include "Kismet/GameplayStatics.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

// Sets default values
ACloth::ACloth()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ClothMesh = CreateDefaultSubobject<UProceduralMeshComponent>("Procedural Mesh");
	ClothMesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ACloth::BeginPlay()
{
	Super::BeginPlay();

    ClothMesh->SetMaterial(0, ClothMaterial);

    // Find sphere
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AClothSphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        Sphere = Cast<AClothSphere>(FoundActors[0]);
    }

	CreateParticles();
	CreateConstraints();

    GenerateMesh();

    GetWorldTimerManager().SetTimer(UpdateTimer, this, &ACloth::Update, TimeStep, true);
}

void ACloth::Destroyed()
{
    CleanUp();

	Super::Destroyed();
}

void ACloth::ShuffleConstraintIndices()
{
    for (int i = ShuffledConstraintIndices.Num() - 1; i > 0; i--)
    {
        int j = FMath::FloorToInt(FMath::SRand() * (i + 1)) % Constraints.Num();

        int temp = ShuffledConstraintIndices[i];
        ShuffledConstraintIndices[i] = ShuffledConstraintIndices[j];
        ShuffledConstraintIndices[j] = temp;
    }

}

void ACloth::AddRandomBurn()
{
    int iRandHorz = FMath::RandRange(0, NumHorzParticles - 1);
    int iRandVert = FMath::RandRange(0, NumVertParticles - 1);

    Particles[iRandVert][iRandHorz]->AddBurn(0.25f);

}

void ACloth::DeleteRandomConstraint()
{
    /*
    int iRandom = FMath::RandRange(0, Constraints.Num() - 1);

    Constraints[iRandom]->DisableConstraint();
    */
    int iRandHorz = FMath::RandRange(0, NumHorzParticles - 1);
    int iRandVert = 10;

    Particles[iRandVert][iRandHorz]->DeleteFirstConstraint();
}

// Called every frame
void ACloth::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    CalculateWindVector();

    GenerateMesh();
}

void ACloth::CreateParticles()
{
    HorzDist = ClothWidth / (NumHorzParticles - 1);
    VertDist = ClothHeight / (NumVertParticles - 1);

    FVector StartPos(0);
    StartPos.X = -ClothWidth / 2;
    StartPos.Z = ClothHeight / 2;

    for (int Vert = 0; Vert < NumVertParticles; Vert++)
    {
        TArray<ClothParticle*> ParticleRow;

        for (int Horz = 0; Horz < NumHorzParticles; Horz++)
        {
            FVector ParticlePos = { StartPos.X + Horz * HorzDist, StartPos.Y, StartPos.Z - Vert * VertDist };
            ClothParticle* NewParticle = new ClothParticle(ParticlePos);

            // Pin top row only, always pin left and right most particle, pin every particle until desired hooks
            int NumInteriorHooks = NumHooks - 2;

            bool ShouldPin = false;
            for (int i = 0; i < NumInteriorHooks; i++)
            {
                float Percent = 1.0f / (NumInteriorHooks + 1);
                Percent *= i + 1;
                Percent *= NumHorzParticles - 1;
                int PinnedIndex = FMath::RoundToInt(Percent);

                if (PinnedIndex == Horz)
                {
                    ShouldPin = true;
                    break;
                }
            }

            bool Pinned = Vert == 0 && (Horz == 0 || Horz == NumHorzParticles - 1 || ShouldPin);
            NewParticle->SetPinned(Pinned);

            ParticleRow.Add(NewParticle);
        }

        Particles.Add(ParticleRow);
    }
}

void ACloth::CreateConstraints()
{
    // Clear old array of constraint indices
    ShuffledConstraintIndices.Empty();

    int ConstraintIndex = 0;

    for (int Vert = 0; Vert < NumVertParticles; Vert++)
    {
        for (int Horz = 0; Horz < NumHorzParticles; Horz++)
        {
            ClothConstraint* VerticalInterwoven = nullptr;

            if (Vert < NumVertParticles - 2)
            {
                // Make a vertical interwoven constraint
                VerticalInterwoven = new ClothConstraint(Particles[Vert][Horz], Particles[Vert + 2][Horz]);

                // Set as an interwoven constraint
                VerticalInterwoven->SetInterwoven(true);

                ShuffledConstraintIndices.Add(ConstraintIndex);
                ConstraintIndex++;
            }

            if (Vert < NumVertParticles - 1)
            {
                // Make a vertical constraint
                ClothConstraint* NewConstraint = new ClothConstraint(Particles[Vert][Horz], Particles[Vert + 1][Horz]);
                Constraints.Add(NewConstraint);

                Particles[Vert][Horz]->AddConstraint(NewConstraint);
                Particles[Vert + 1][Horz]->AddConstraint(NewConstraint);

                if (VerticalInterwoven)
                {
                    NewConstraint->SetAssociatedInterwovenConstraint(VerticalInterwoven);
                    Constraints.Add(VerticalInterwoven);

                    Particles[Vert][Horz]->AddConstraint(VerticalInterwoven);
                    Particles[Vert + 2][Horz]->AddConstraint(VerticalInterwoven);
                }

                ShuffledConstraintIndices.Add(ConstraintIndex);
                ConstraintIndex++;
            }

            ClothConstraint* HorziontalInterwoven = nullptr;

            if (Horz < NumHorzParticles - 2)
            {
                // Make a horizontal interwoven constraint
                HorziontalInterwoven = new ClothConstraint(Particles[Vert][Horz], Particles[Vert][Horz + 2]);

                // Set as an interwoven constraint
                HorziontalInterwoven->SetInterwoven(true);

                ShuffledConstraintIndices.Add(ConstraintIndex);
                ConstraintIndex++;
            }

            if (Horz < NumHorzParticles - 1)
            {
                // Make a horizontal constraint
                ClothConstraint* NewConstraint = new ClothConstraint(Particles[Vert][Horz], Particles[Vert][Horz + 1]);
                Constraints.Add(NewConstraint);

                Particles[Vert][Horz]->AddConstraint(NewConstraint);
                Particles[Vert][Horz + 1]->AddConstraint(NewConstraint);

                if (HorziontalInterwoven)
                {
                    NewConstraint->SetAssociatedInterwovenConstraint(HorziontalInterwoven);
                    Constraints.Add(HorziontalInterwoven);

                    Particles[Vert][Horz]->AddConstraint(HorziontalInterwoven);
                    Particles[Vert][Horz + 2]->AddConstraint(HorziontalInterwoven);
                }

                ShuffledConstraintIndices.Add(ConstraintIndex);
                ConstraintIndex++;
            }
        }
    }

    if (InitialiseRandomConstraintOrder)
    {
        ShuffleConstraintIndices();
    }
}

void ACloth::GenerateMesh()
{
    ClothVertices.Reset();
    ClothTriangles.Reset();
    ClothNormals.Reset();
    ClothUVs.Reset();
    ClothColors.Reset();

    for (int Vert = 0; Vert < NumVertParticles; Vert++)
    {
        for (int Horz = 0; Horz < NumHorzParticles; Horz++)
        {
            ClothVertices.Add(Particles[Vert][Horz]->GetPosition());

            // For vertex colour use burn amount
            FLinearColor ParticleColour(Particles[Vert][Horz]->GetBurn(), 0.0f, 0.0f, 0.0f);
            ClothColors.Add(ParticleColour);

            ClothUVs.Add(FVector2D(float(Horz) / (NumHorzParticles - 1), float(Vert) / (NumVertParticles - 1)));
        }
    }

    for (int Vert = 0; Vert < NumVertParticles - 1; Vert++)
    {
        for (int Horz = 0; Horz < NumHorzParticles - 1; Horz++)
        {
            TryCreateTriangles(Particles[Vert][Horz], Particles[Vert][Horz + 1],
                Particles[Vert + 1][Horz], Particles[Vert + 1][Horz + 1],
                Vert * NumHorzParticles + Horz);
        }
    }

    TArray<FProcMeshTangent> ClothTangents;

    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(ClothVertices, ClothTriangles, ClothUVs, ClothNormals, ClothTangents);

    ClothMesh->CreateMeshSection_LinearColor(0, ClothVertices, ClothTriangles, ClothNormals, ClothUVs, ClothColors, ClothTangents, false);
}

void ACloth::CheckForCollision()
{
    for (int Vert = 0; Vert < NumVertParticles; Vert++)
    {
        TArray<ClothParticle*> ParticleRow;

        for (int Horz = 0; Horz < NumHorzParticles; Horz++)
        {
            // Check for sphere collision
            if (Sphere)
            {
                FVector SphereCenter = Sphere->GetActorLocation();
                float SphereRadius = Sphere->GetCollisionRadius(); 

                ClothParticle* Particle = Particles[Vert][Horz];
                FVector ParticlePosition = Particle->GetPosition();

                FVector Offset = ParticlePosition - SphereCenter;
                float Distance = Offset.Size();

                if (Distance < SphereRadius)
                {
                    FVector Correction = Offset.GetSafeNormal() * (SphereRadius - Distance);
                    Particle->SetPosition(ParticlePosition + Correction);

                    // Apply velocity dampening to simulate realistic collision response
                    FVector Velocity = Particle->GetPosition() - Particle->GetOldPosition();
                    Velocity *= 0.8f;
                    Particle->SetVelocity(Velocity);
                    UE_LOG(LogTemp, Warning, TEXT("Collision detected for particle at %s"), *Particle->GetPosition().ToString());
                }
            }

            // Check for capsule collision

            // Check for ground collision
            Particles[Vert][Horz]->CheckForGroundCollision(GroundHeight - ClothMesh->GetComponentLocation().Z);
        }
    }

}

void ACloth::CleanUp()
{
    for (int Vert = 0; Vert < Particles.Num(); Vert++)
    {
        for (int Horz = 0; Horz < Particles[Vert].Num(); Horz++)
        {
            delete Particles[Vert][Horz];
        }
    }

    for (auto iter : Constraints)
    {
        delete iter;
    }

    Particles.Empty();
    Constraints.Empty();
}

void ACloth::ResetCloth()
{
    CleanUp();

    CreateParticles();
    CreateConstraints();
}

void ACloth::ConstrictCloth(float _constrictedAmount)
{
    float constrictedWidth = ClothWidth * _constrictedAmount;
    float constrictedDist = constrictedWidth / (NumHorzParticles - 1);

    FVector StartPos(0);
    StartPos.X = -constrictedWidth / 2;
    StartPos.Z = ClothHeight / 2;

    for (int Horz = 0; Horz < NumHorzParticles; Horz++)
    {
        FVector ParticlePos = { StartPos.X + Horz * constrictedDist, StartPos.Y, StartPos.Z };

        if (Particles[0][Horz]->GetPinned())
        {
            Particles[0][Horz]->SetPosition(ParticlePos);
        }
    }

}

FVector ACloth::GetParticleNormal(int _XIndex, int _YIndex)
{
    return ClothNormals[_XIndex + _YIndex * NumHorzParticles];
}

void ACloth::CalculateWindVector()
{
    WindVector = WindRotation.Vector();

    WindVector.Normalize();

    float WindStrength = FMath::Lerp(200.0f, 1000.0f, (FMath::Sin(GetGameTimeSinceCreation() * WindOscillationFrequency) + 1.0f) * 0.5f);
    float WindStrength2 = FMath::Lerp(200.0f, 1000.0f, (FMath::Sin(GetGameTimeSinceCreation() * WindOscillationFrequency2) + 1.0f) * 0.5f);
    WindVector *= (WindStrength + WindStrength2) * WindMultiplier;
}

void ACloth::ReleaseCloth()
{
    for (int Horz = 0; Horz < NumHorzParticles; Horz++)
    {
        Particles[0][Horz]->SetPinned(false);
    }
}

void ACloth::TryCreateTriangles(ClothParticle* _topLeft, ClothParticle* _topRight, ClothParticle* _bottomLeft,
                                ClothParticle* _bottomRight, int _topLeftIndex)
{
    int TopLeftIndex = _topLeftIndex;
    int TopRightIndex = _topLeftIndex + 1;
    int BottomLeftIndex = _topLeftIndex + NumHorzParticles;
    int BottomRightIndex = _topLeftIndex + NumHorzParticles + 1;

    if (_topLeft->SharesConstraint(_topRight) && _topLeft->SharesConstraint(_bottomLeft))
    {
        ClothTriangles.Add(TopLeftIndex);
        ClothTriangles.Add(TopRightIndex);
        ClothTriangles.Add(BottomLeftIndex);

        if (_bottomRight->SharesConstraint(_topRight) && _bottomRight->SharesConstraint(_bottomLeft))
        {
            ClothTriangles.Add(TopRightIndex);
            ClothTriangles.Add(BottomRightIndex);
            ClothTriangles.Add(BottomLeftIndex);
        }
    }
    else if (_bottomLeft->SharesConstraint(_topLeft) && _bottomLeft->SharesConstraint(_bottomRight))
    {
        ClothTriangles.Add(BottomLeftIndex);
        ClothTriangles.Add(TopLeftIndex);
        ClothTriangles.Add(BottomRightIndex);

        if (_topRight->SharesConstraint(_bottomRight) && _topRight->SharesConstraint(_topLeft))
        {
            ClothTriangles.Add(TopRightIndex);
            ClothTriangles.Add(BottomRightIndex);
            ClothTriangles.Add(TopLeftIndex);
        }
    }
}

void ACloth::Update()
{
    float iterationTimeStep = TimeStep / (float)VerletIntegrationIterations;
    float divStep = 1.0f / (float)VerletIntegrationIterations;

    // Iteratively process particles and constraints
    for (int i = 0; i < VerletIntegrationIterations; i++)
    {
        // Update all particles
        for (int Vert = 0; Vert < NumVertParticles; Vert++)
        {
            for (int Horz = 0; Horz < NumHorzParticles; Horz++)
            {
                // Apply gravity to each particle (considering mass if needed)
                Particles[Vert][Horz]->AddForce(FVector(0.0f, 0.0f, -981.0f * iterationTimeStep));

                // Apply wind
                FVector ParticleNormal = GetParticleNormal(Horz, Vert);

                FVector NormalWind = WindVector;
                NormalWind.Normalize();

                // Abs dot product so what matters is alignment with the normal
                float WindAlignment = FMath::Abs(NormalWind.Dot(ParticleNormal));

                Particles[Vert][Horz]->AddForce(WindVector * WindAlignment * iterationTimeStep);

                // Update particle position
                Particles[Vert][Horz]->Update(iterationTimeStep);
            }
        }

        // Check for collisions after updating particles
        CheckForCollision();

        // Shuffle constraints if needed
        if (UseRandomConstraintOrder && i == 0) // Shuffle only once per update call
        {
            ShuffleConstraintIndices();
        }

        // Update all constraints
        for (int j = 0; j < ShuffledConstraintIndices.Num(); j++)
        {
            // Skip interwoven constraints if not simulating them
            if (Constraints[ShuffledConstraintIndices[j]]->GetIsInterwoven() && !SimulateInterwovenConstraints)
            {
                continue;
            }

            Constraints[ShuffledConstraintIndices[j]]->Update(divStep);
        }

        // Check collisions again after constraints to ensure stability
        CheckForCollision();
    }
}
