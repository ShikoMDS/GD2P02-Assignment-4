// Fill out your copyright notice in the Description page of Project Settings.


#include "ClothParticle.h"
#include "ClothConstraint.h"

ClothParticle::ClothParticle(FVector _position)
{
	Position = _position;
	OldPosition = Position;
}

ClothParticle::~ClothParticle()
{
}

void ClothParticle::AddConstraint(ClothConstraint* _constraint)
{
	Constraints.Add(_constraint);
}

bool ClothParticle::SharesConstraint(ClothParticle* _otherParticle)
{
	for (auto iter : _otherParticle->GetConstraints())
	{
		if (Constraints.Contains(iter))
		{
			return true;
		}
	}

	return false;
}

TArray<ClothConstraint*> ClothParticle::GetConstraints()
{
	return Constraints;
}

bool ClothParticle::GetPinned()
{
	return IsPinned;
}

void ClothParticle::SetPinned(bool _Pin)
{
	IsPinned = _Pin;
}

FVector ClothParticle::GetPosition()
{
	return Position;
}

void ClothParticle::OffsetPosition(FVector _offset)
{
	Position += _offset;
}

void ClothParticle::AddAcceleration(FVector _Force)
{
	if (!GetPinned())
	{
		Acceleration += _Force;
	}
}

void ClothParticle::Update(float _DeltaTime)
{
	FVector cachedPosition = Position;

	if (OldDeltaTime <= 0.0f)
	{
		OldDeltaTime = _DeltaTime;
	}

	Position = Position + ((Position - OldPosition) * (_DeltaTime / OldDeltaTime)) +
		(Acceleration * _DeltaTime * ((_DeltaTime + OldDeltaTime) * 0.5f));

	OldPosition = cachedPosition;
	Acceleration = { 0, 0, 0 };
	OldDeltaTime = _DeltaTime;
}
