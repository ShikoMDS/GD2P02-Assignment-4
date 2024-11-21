// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class CLOTHSIMULATION_API ClothConstraint
{
public:
	ClothConstraint(class ClothParticle* _particleA, ClothParticle* _particleB);
	~ClothConstraint();

	void Update(float _DeltaTime);

	void SetInterwoven(bool _interwoven);
	bool GetIsInterwoven();

	void DisableConstraint();

	bool IsEnabled();

	void SetAssociatedInterwovenConstraint(ClothConstraint* _constraint);

private:

	ClothParticle* ParticleA = nullptr;
	ClothParticle* ParticleB = nullptr;

	ClothConstraint* InterwovenConstraint = nullptr;

	float RestingDistance;

	bool IsInterwoven = false;

	bool Enabled = true;
};
