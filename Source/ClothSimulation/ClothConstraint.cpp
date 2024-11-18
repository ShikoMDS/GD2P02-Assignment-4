// Fill out your copyright notice in the Description page of Project Settings.


#include "ClothConstraint.h"
#include "ClothParticle.h"

ClothConstraint::ClothConstraint(ClothParticle* _particleA, ClothParticle* _particleB)
{
    ParticleA = _particleA;
    ParticleB = _particleB;
}


ClothConstraint::~ClothConstraint()
{
}
