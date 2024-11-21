// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClothSphere.generated.h"

UCLASS()
class CLOTHSIMULATION_API AClothSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AClothSphere();

	UFUNCTION(BlueprintCallable, Category = "Collision")
	float GetCollisionRadius();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionRadius = 100.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
