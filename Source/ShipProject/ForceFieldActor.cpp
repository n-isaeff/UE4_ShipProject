// Fill out your copyright notice in the Description page of Project Settings.


#include "ForceFieldActor.h"

// Sets default values
AForceFieldActor::AForceFieldActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(FName("ForceFieldActor"));

}

FVector AForceFieldActor::GetForce(const FVector2D& pos) const
{
	return FVector();
}

void AForceFieldActor::UpdateForces()
{
	if (m_lastUpdFrame < GFrameNumber)
	{
		RecalculateForces();
	}
	m_lastUpdFrame = GFrameNumber;
}

// Called when the game starts or when spawned
void AForceFieldActor::BeginPlay()
{
	PrimaryActorTick.bCanEverTick = DynamicField;
	Super::BeginPlay();
	
}

void AForceFieldActor::RecalculateForces()
{
}

// Called every frame
void AForceFieldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

