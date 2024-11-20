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

void ClothParticle::AddForce(FVector _Force)
{
		Acceleration += _Force;
}

void ClothParticle::Update(float _DeltaTime)
{
	if (GetPinned() || OnGround)
	{
		Acceleration = {0, 0, 0};
		return;
	}

	FVector cachedPosition = Position;

	if (OldDeltaTime <= 0.0f)
	{
		OldDeltaTime = _DeltaTime;
	}

	/*
	// Framerate independent verlet integration
	Position = Position + 
		((Position - OldPosition) * ((1.0f - Damping) * (_DeltaTime / OldDeltaTime))) +
		(Acceleration * _DeltaTime * ((_DeltaTime + OldDeltaTime) * 0.5f)); 
	*/
	
	// Non-framerate independent
	Position = Position +
		(Position - OldPosition) * (1.0f - Damping * _DeltaTime) + 
		Acceleration * _DeltaTime;
	

	Acceleration = { 0, 0, 0 };

	OldPosition = cachedPosition;
	OldDeltaTime = _DeltaTime;
}

void ClothParticle::CheckForGroundCollision(float _groundHeight)
{
	if (Position.Z <= _groundHeight + 1)
	{
		OnGround = true;

		if (Position.Z <= _groundHeight)
		{
			Position.Z = _groundHeight;
		}
	}
	else
	{
		OnGround = false;
	}
}
