// Fill out your copyright notice in the Description page of Project Settings.


#include "Cloth.h"
#include "ClothParticle.h"
#include "ClothConstraint.h"
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
    for (int Vert = 0; Vert < NumVertParticles; Vert++)
    {
        for (int Horz = 0; Horz < NumHorzParticles; Horz++)
        {
            if (Vert < NumVertParticles - 1)
            {
                // Make a vertical constraint
                ClothConstraint* NewConstraint = new ClothConstraint(Particles[Vert][Horz], Particles[Vert + 1][Horz]);
                Constraints.Add(NewConstraint);

                Particles[Vert][Horz]->AddConstraint(NewConstraint);
                Particles[Vert + 1][Horz]->AddConstraint(NewConstraint);
            }

            if (Horz < NumHorzParticles - 1)
            {
                // Make a horizontal constraint
                ClothConstraint* NewConstraint = new ClothConstraint(Particles[Vert][Horz], Particles[Vert][Horz + 1]);
                Constraints.Add(NewConstraint);

                Particles[Vert][Horz]->AddConstraint(NewConstraint);
                Particles[Vert][Horz + 1]->AddConstraint(NewConstraint);
            }
        }
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
            ClothColors.Add(FLinearColor::Black);
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
            // Check for ground collision
            Particles[Vert][Horz]->CheckForGroundCollision(GroundHeight - ClothMesh->GetComponentLocation().Z);

            // Check for sphere collision

            // Check for capsule collision
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

    // Iterating particle first
    
    // Update all particles
    for (int Vert = 0; Vert < NumVertParticles; Vert++)
    {
        TArray<ClothParticle*> ParticleRow;

        for (int Horz = 0; Horz < NumHorzParticles; Horz++)
        {
            // Apply gravity to each particle (we could consider mass too)
            Particles[Vert][Horz]->AddForce(FVector(0.0f, 0.0f, -981.0f * 1.0f * TimeStep));

            // Apply wind
            FVector ParticleNormal = GetParticleNormal(Horz, Vert);

            FVector NormalWind = WindVector;
            NormalWind.Normalize();

            // Abs dot product so what matters is we're perpendicular
            float WindAlignment = FMath::Abs(NormalWind.Dot(ParticleNormal));

            Particles[Vert][Horz]->AddForce(WindVector * WindAlignment * TimeStep);

            Particles[Vert][Horz]->Update(TimeStep);
        }
    }

    // Iterating constraints second 
    for (int i = 0; i < VerletIntegrationIterations; i++)
    {
	    // Update all constraints
    	for (auto iter : Constraints)
    	{
    		iter->Update(divStep);
    	}
    }

    // Check for collision
    CheckForCollision();
}

/*
void ACloth::Update()
{
    float iterationTimeStep = TimeStep / (float)UpdateSteps;
    // Iterate over the particles some number of times
    for (int i = 0; i < UpdateSteps; i++)
    {
	    // Update all constraints
    	for (auto iter : Constraints)
    	{
    		iter->Update(TimeStep);
    	}

    	// Update all particles
    	for (int Vert = 0; Vert < Particles.Num(); Vert++)
    	{
    		for (int Horz = 0; Horz < Particles[Vert].Num(); Horz++)
    		{
    			int index = Horz + Particles[Vert].Num() * Vert;
    			FVector windForce = {2, 10, 0};
    			float dotProduct = FVector::DotProduct(ClothNormals[index], windForce);
    			Particles[Vert][Horz]->AddForce({ 0, 0, -100});
    			Particles[Vert][Horz]->AddForce(windForce * abs(dotProduct));
    			Particles[Vert][Horz]->Update(TimeStep);
    		}
    	}
    }
}
*/