// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Kismet/GameplayStatics.h"

#include "ForceFieldActor.generated.h"

UCLASS()
class SHIPPROJECT_API AForceFieldActor : public AActor
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "Force Field Base")
	bool DynamicField;

public:	
	// Sets default values for this actor's properties
	AForceFieldActor();

	FVector GetForce(const FVector2D& pos) const;
	virtual void UpdateForces();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void RecalculateForces();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	uint64 m_lastUpdFrame = 0;

};
