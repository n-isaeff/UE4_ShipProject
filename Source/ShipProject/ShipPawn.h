// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
//

#include "ShipPawn.generated.h"

UCLASS()
class SHIPPROJECT_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShipPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FVector2D GetWindForce() const;
	FVector2D GetFlowForce() const;

	float WaterLevelPerPoint(const FVector& point, bool& found, const float searchRange = 500);
	float CalculateWaterlineAvr();
	void CalculateBuoyantForces();

	void RotateCamera();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void TurnRudder(float val);
	void TurnSail(float val);

	void CameraPitch(float val);
	void CameraYaw(float val);
	void CameraZoom(float val);

private:
	void TurnShipPart(UStaticMeshComponent* part, float degrees);

public:
	UPROPERTY(EditAnywhere, Category = "Ship Controls")
	float RudderAngle{ 0 };

	UPROPERTY(EditAnywhere, Category = "Ship Controls")
	float SailAngle{ 0 };

	UPROPERTY(EditAnywhere, Category = "Ship Mesh")
	UStaticMeshComponent* ShipHull;

	UPROPERTY(EditAnywhere, Category = "Ship Mesh")
	UStaticMeshComponent* ShipRudder;

	UPROPERTY(EditAnywhere, Category = "Ship Mesh")
	float ShipWaterLine = 100;

	UPROPERTY(EditAnywhere, Category = "Ship Mesh")
	TArray<UStaticMeshComponent*> ShipSails;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	USceneComponent* SpringArmTarget;

	UPROPERTY(EditAnywhere)
	USceneComponent* ShipRoot;

	UPROPERTY(EditAnywhere, Category = "Ship Debug")
	FVector2D WindSpeed{ 0, 0 };

	UPROPERTY(EditAnywhere, Category = "Ship Debug")
	FVector2D FlowSpeed{ 0, 0 };

	UPROPERTY(EditAnywhere, Category = "Ship Physics")
	TMap<FString, USceneComponent *> BuoyancyPoints;

	UPROPERTY(EditAnywhere, Category = "Ship Physics")
	TMap<FString, float> BuoyancyPointsWaterline;

	UPROPERTY(EditAnywhere)
	bool ShipPlayer = true;

private:
	int m_sailsNum = 5;
	//float m_vertSpeed{ 0 };
	FVector m_rotSpeed{ 0, 0, 0 };
	float m_hullSideA = 0, m_hullSideB = 0;

	FVector2D m_cameraOrient = { 0, 0 };
	float m_cameraZoom = 1;

	TArray<FString> m_bNames; 

	class AProceduralSeaActor* m_seaInstance = nullptr;
	
};
