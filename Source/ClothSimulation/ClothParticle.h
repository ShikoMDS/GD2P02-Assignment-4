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

	FVector GetPosition();

	void OffsetPosition(FVector _offset);

private:

	FVector Position = { 0, 0, 0 };
	TArray<class ClothConstraint*> Constraints;
};
