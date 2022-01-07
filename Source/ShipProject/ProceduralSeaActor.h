// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ProceduralMeshComponent.h"

#include "ProceduralSeaActor.generated.h"

USTRUCT()
struct SHIPPROJECT_API FUProceduralSeaTile
{
	GENERATED_BODY()
public:
	FUProceduralSeaTile(AActor* parent, int idx);
	FUProceduralSeaTile();

	void CreateTile(const FVector& aa, const FVector& bb, FIntPoint divs, const FVector2D& startPos, const float noiseScale, const float noiseStrength);

	void UpdateTileSubdivs(FIntPoint divs, const float noiseScale, const float noiseStrength, const FVector2D& noisePos);
	void UpdateTileNoise(const float noiseScale, const float noiseStrength, const FVector2D& noisePos, const FVector2D& drift);

	float GetWaveHeight(const FVector2D& at, float noiseStrength, float noiseScale, const FVector2D& noisePos, const FVector2D& drift);
public:

	UPROPERTY(EditAnywhere, Category = "Sea Mesh")
	UProceduralMeshComponent* ProceduralSeaTile;

	UPROPERTY()
	TArray<FVector> Verts;
	UPROPERTY()
	TArray<FVector> Norms;
	UPROPERTY()
	TArray<FVector2D> UVs;
	UPROPERTY()
	TArray<int32> Tries;

private:
	FVector m_aa, m_bb;
	FVector2D m_startPos;
	FIntPoint m_divs;

};

UCLASS()
class SHIPPROJECT_API AProceduralSeaActor : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AProceduralSeaActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, Category = "Sea Mesh")
	TArray<FUProceduralSeaTile> SeaTiles;

	UPROPERTY(EditAnywhere, Category = "Sea General")
	USceneComponent* SeaRoot;

	UPROPERTY(EditAnywhere, Category = "Sea Mesh")
	FIntPoint Subdivisions = { 5, 5 };

	UPROPERTY(EditAnywhere, Category = "Sea Mesh")
	FIntPoint TileNumber = { 3, 3 };

	UPROPERTY(EditAnywhere, Category = "Sea Mesh")
	FVector2D Size = { 1500, 1500 };

	UPROPERTY(EditAnywhere, Category = "Sea Mesh")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "Sea Noise")
	float NoiseScale = 0.0025f;

	UPROPERTY(EditAnywhere, Category = "Sea Noise")
	float NoiseStrength = 30;

	UPROPERTY(EditAnywhere, Category = "Sea Noise")
	FVector2D Drift = { 0.5f, 0 };

	UPROPERTY(EditAnywhere, Category = "Sea Mesh")
	float SmallNoiseDrift = 1;

private:
	FVector2D m_currentNoisePos = { 0, 0 };
	UMaterialInstanceDynamic* m_materialInstance = nullptr;
	int m_centerTile = 0;

	AActor* m_shipPlayer = nullptr;
};
