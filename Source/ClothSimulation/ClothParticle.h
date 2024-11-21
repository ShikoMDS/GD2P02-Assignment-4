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

	FVector GetOldPosition();

	void SetPosition(FVector _NewPos);

	void SetVelocity(FVector _newVelocity);

	void OffsetPosition(FVector _offset);

	void AddForce(FVector _Force);

	void Update(float _DeltaTime);

	void CheckForGroundCollision(float _groundHeight);

	void AddBurn(float _burnAmount);

	float GetBurn();

	void RemoveConstraint(ClothConstraint*);

	void DeleteFirstConstraint();

private:

	FVector Position = { 0, 0, 0 };
	FVector OldPosition = { 0, 0, 0 };
	float OldDeltaTime = -1.0f;
	FVector Acceleration = { 0, 0, 0 };
	TArray<class ClothConstraint*> Constraints;
	bool IsPinned = false;
	float Damping = 0.0f;
	bool OnGround = false;
	float BurnAmount = 0.0f;
	FVector Velocity = {0, 0, 0};
};
