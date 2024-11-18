// Fill out your copyright notice in the Description page of Project Settings.


#include "ClothConstraint.h"
#include "ClothParticle.h"

ClothConstraint::ClothConstraint(ClothParticle* _particleA, ClothParticle* _particleB)
{
    ParticleA = _particleA;
    ParticleB = _particleB;

    RestingDistance = FVector::Dist(ParticleA->GetPosition(), ParticleB->GetPosition());
}


ClothConstraint::~ClothConstraint()
{
}

void ClothConstraint::Update(float _DeltaTime)
{
    FVector CurrentOffset = ParticleB->GetPosition() - ParticleA->GetPosition();

    FVector Correction = CurrentOffset * (1.0f - RestingDistance / CurrentOffset.Size());
    FVector HalfCorrection = Correction * 0.5f;

    ParticleA->OffsetPosition(HalfCorrection);
    ParticleB->OffsetPosition(-HalfCorrection);
}
