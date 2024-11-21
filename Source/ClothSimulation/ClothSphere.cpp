// Fill out your copyright notice in the Description page of Project Settings.


#include "ClothSphere.h"

// Sets default values
AClothSphere::AClothSphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

float AClothSphere::GetCollisionRadius()
{
	return CollisionRadius;
}

// Called when the game starts or when spawned
void AClothSphere::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AClothSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

