// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class CLOTHSIMULATION_API ClothParticle
{
public:
	ClothParticle(FVector _position);
	~ClothParticle();

	void AddConstraint(class ClothConstraint* _constraint);

	bool SharesConstraint(ClothParticle* _otherParticle);

	TArray<class ClothConstraint*> GetConstraints();

	bool GetPinned();
	void SetPinned(bool _Pin);

	FVector GetPosition();

	void OffsetPosition(FVector _offset);

	void AddAcceleration(FVector _Force);

	void Update(float _DeltaTime);

private:

	FVector Position = { 0, 0, 0 };
	FVector OldPosition = { 0, 0, 0 };
	float OldDeltaTime = -1.0f;
	FVector Acceleration = { 0, 0, 0 };
	TArray<class ClothConstraint*> Constraints;
	bool IsPinned = false;
};
