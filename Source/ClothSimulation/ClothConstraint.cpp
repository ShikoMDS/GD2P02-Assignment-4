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
    if (!Enabled) return;

    FVector CurrentOffset = ParticleB->GetPosition() - ParticleA->GetPosition();

    /*
    float length = (float)CurrentOffset.Size();
    float stiffness = 800.0f;
    float deform = length - RestingDistance;
    CurrentOffset.Normalize();

    FVector force = CurrentOffset * stiffness * deform;
    FVector halfForce = force * 0.5f;
    */

    FVector Correction = CurrentOffset * (1.0f - RestingDistance / CurrentOffset.Size());
    FVector HalfCorrection = Correction * 0.5f;

    if (!ParticleA->GetPinned() && !ParticleB->GetPinned())
    {
        ParticleA->OffsetPosition(HalfCorrection);
        ParticleB->OffsetPosition(-HalfCorrection);
        //ParticleA->AddAcceleration(halfForce);
        //ParticleB->AddAcceleration(-halfForce);
    }
    else if (!ParticleA->GetPinned())
    {
        ParticleA->OffsetPosition(Correction);
        //ParticleA->AddAcceleration(force);
    }
    else if (!ParticleB->GetPinned())
    {
        ParticleB->OffsetPosition(-Correction);
        //ParticleB->AddAcceleration(-force);
    }

}

void ClothConstraint::SetInterwoven(bool _interwoven)
{
    IsInterwoven = _interwoven;
}

bool ClothConstraint::GetIsInterwoven()
{
    return IsInterwoven;
}

void ClothConstraint::DisableConstraint()
{
    ParticleA->RemoveConstraint(this);
    ParticleB->RemoveConstraint(this);

    if (InterwovenConstraint) InterwovenConstraint->DisableConstraint();

    Enabled = false;
}

bool ClothConstraint::IsEnabled()
{
    return Enabled;
}

void ClothConstraint::SetAssociatedInterwovenConstraint(ClothConstraint* _constraint)
{
    InterwovenConstraint = _constraint;
}
