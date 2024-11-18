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

private:

	ClothParticle* ParticleA = nullptr;
	ClothParticle* ParticleB = nullptr;
};
