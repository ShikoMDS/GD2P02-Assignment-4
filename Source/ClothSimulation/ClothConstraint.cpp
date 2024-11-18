// Fill out your copyright notice in the Description page of Project Settings.


#include "ClothConstraint.h"
#include "ClothParticle.h"

ClothConstraint::ClothConstraint(ClothParticle* _particleA, ClothParticle* _particleB)
{
    ParticleA = _particleA;
    ParticleB = _particleB;

    RestingDistance = FVector::Dist(ParticleA->GetPosition(), ParticleB->GetPosition()) * 1.1f;
}


ClothConstraint::~ClothConstraint()
{
}

void ClothConstraint::Update(float _DeltaTime)
{
    if (ParticleA->GetPinned() && ParticleB->GetPinned())
    {
        return;
    }

    FVector CurrentOffset = ParticleB->GetPosition() - ParticleA->GetPosition();

    FVector Correction = CurrentOffset * (1.0f - RestingDistance / CurrentOffset.Size());
    FVector HalfCorrection = Correction * 0.5f;

    if (!ParticleA->GetPinned() && !ParticleB->GetPinned())
    {
        ParticleA->OffsetPosition(HalfCorrection);
        ParticleB->OffsetPosition(-HalfCorrection);
    }
    else if (!ParticleA->GetPinned())
    {
        ParticleA->OffsetPosition(Correction);
    }
    else if (!ParticleB->GetPinned())
    {
        ParticleB->OffsetPosition(-Correction);
    }

    return;

    // IF WE WANT TO DO WITH FORCES
    float length = (float)CurrentOffset.Size();
    float stiffness = 80000.0f;
    float deform = length - RestingDistance;
    FVector normOffset = CurrentOffset;
    normOffset.Normalize();

    FVector force = normOffset * stiffness * deform;
    FVector halfForce = force * 0.5f;

    if (!ParticleA->GetPinned() && !ParticleB->GetPinned())
    {
        ParticleA->AddAcceleration(halfForce);
        ParticleB->AddAcceleration(-halfForce);
    }
    else if (!ParticleA->GetPinned())
    {
        ParticleA->AddAcceleration(force);
    }
    else if (!ParticleB->GetPinned())
    {
        ParticleB->AddAcceleration(-force);
    }

}
