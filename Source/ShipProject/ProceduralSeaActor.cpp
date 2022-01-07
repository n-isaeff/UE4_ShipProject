// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralSeaActor.h"
#include "KismetProceduralMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "ShipPawn.h"

// Sets default values
AProceduralSeaActor::AProceduralSeaActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SeaRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SeaRoot"));
	SetRootComponent(SeaRoot);
	SeaTiles.Empty();
	for (int i = 0; i < TileNumber.X; i++)
	{
		for (int j = 0; j < TileNumber.Y; j++)
		{
			SeaTiles.Add(FUProceduralSeaTile());
			FUProceduralSeaTile& tile = SeaTiles[i * TileNumber.X + j];
			FString tileName = FString("SeaTile") + FString::FromInt(i * TileNumber.X + j);
			tile.ProceduralSeaTile = CreateDefaultSubobject<UProceduralMeshComponent>(FName(tileName));
			tile.ProceduralSeaTile->SetupAttachment(SeaRoot);
			/*FString tileName = FString("SeaTile") + FString::FromInt(i * TileNumber.X + j);
			SeaTiles.Add(CreateDefaultSubobject<UProceduralMeshComponent>(FName(tileName)));*/
		}
	}
	Tags.Add(FName("CurrentSea"));
}

// Called when the game starts or when spawned
void AProceduralSeaActor::BeginPlay()
{
	Super::BeginPlay();
	if (Material && !m_materialInstance)
	{
		m_materialInstance = UMaterialInstanceDynamic::Create(Material, this);
		for (auto& t : SeaTiles)
		{
			t.ProceduralSeaTile->SetMaterial(0, m_materialInstance);
		}
	}
}

void AProceduralSeaActor::PostLoad()
{
	Super::PostLoad();
	if (Material)
	{
		m_materialInstance = UMaterialInstanceDynamic::Create(Material, this);
	}
	m_centerTile = TileNumber.X / 2 * TileNumber.X + TileNumber.Y / 2;
	for (int i = 0; i < TileNumber.X; i++)
	{
		for (int j = 0; j < TileNumber.Y; j++)
		{
			FUProceduralSeaTile& tile = SeaTiles[i * TileNumber.X + j];

			FVector
				aa = {		(i - TileNumber.X / 2) * Size.X / (float)TileNumber.X,		(j - TileNumber.Y / 2) * Size.Y / (float)TileNumber.Y, 0 },
				bb = {	(i - TileNumber.X / 2 + 1) * Size.X / (float)TileNumber.X,	(j - TileNumber.Y / 2 + 1) * Size.Y / (float)TileNumber.Y, 0 };
			tile.CreateTile(aa, bb, 
				i == TileNumber.X / 2 && j == TileNumber.Y / 2
				? Subdivisions
				: Subdivisions / 4,
				m_currentNoisePos, NoiseScale, NoiseStrength);
			if (tile.ProceduralSeaTile && m_materialInstance)
				tile.ProceduralSeaTile->SetMaterial(0, m_materialInstance);
		}
	}
}

// Called every frame
void AProceduralSeaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!m_shipPlayer)
	{
		TArray<AActor*> foundActors;
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AShipPawn::StaticClass(), FName(TEXT("PlayerShip")), foundActors);
		if (foundActors.Num() > 0)
		{
			m_shipPlayer = foundActors[0];
		}
	}
	else {
		FVector2D shipPos = FVector2D(((AShipPawn*)m_shipPlayer)->ShipHull->GetComponentLocation());
		SetActorLocation(FVector(shipPos, 0.f));
	}

	m_currentNoisePos += Drift * DeltaTime;
	FLinearColor curShift;
	if (m_materialInstance && Material) 
	{
		FHashedMaterialParameterInfo p(FName(TEXT("Shift")));
		if (m_materialInstance->GetVectorParameterValue(p, curShift))
		{
			m_materialInstance->SetVectorParameterValue(FName(TEXT("Shift")), curShift + FLinearColor(Drift.X, Drift.Y, 0, 0) * DeltaTime * SmallNoiseDrift);
		}
	}
	
	for (size_t i = 0; i < SeaTiles.Num(); i++)
	{
		SeaTiles[i].UpdateTileSubdivs(i == m_centerTile ? Subdivisions : Subdivisions / 4, NoiseScale, NoiseStrength, m_currentNoisePos);
		SeaTiles[i].UpdateTileNoise(NoiseScale, NoiseStrength, m_currentNoisePos, Drift);
	}
}

FUProceduralSeaTile::FUProceduralSeaTile()
{
}

void FUProceduralSeaTile::CreateTile(const FVector& aa, const FVector& bb, FIntPoint divs, const FVector2D& startPos, const float noiseScale, const float noiseStrength)
{
	
	Verts.Empty();
	Tries.Empty();
	Norms.Empty();
	UVs.Empty();
	if (ProceduralSeaTile)
	{
		UKismetProceduralMeshLibrary::CreateGridMeshWelded(divs.X + 1, divs.Y + 1, Tries, Verts, UVs, (bb - aa).X / (float)divs.X);
		ProceduralSeaTile->CreateMeshSection_LinearColor(0, Verts, Tries, Norms, UVs, TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
		ProceduralSeaTile->SetRelativeLocation(aa);
		ProceduralSeaTile->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		m_divs = divs;
		m_startPos = startPos;
		m_aa = aa;
		m_bb = bb;
	}
}

void FUProceduralSeaTile::UpdateTileSubdivs(FIntPoint divs, const float noiseScale, const float noiseStrength, const FVector2D& noisePos)
{
	if (divs != m_divs)
	{
		ProceduralSeaTile->ClearMeshSection(0);
		CreateTile(m_aa, m_bb, divs, noisePos, noiseScale, noiseStrength);
	}
}

void FUProceduralSeaTile::UpdateTileNoise(const float noiseScale, const float noiseStrength, const FVector2D& noisePos, const FVector2D& drift)
{
	for (int32 i = 0; i < Verts.Num(); i++)
	{
		
		//Verts[i].Z = noiseStrength * FMath::PerlinNoise2D(noiseScale * (FVector2D{ Verts[i].X, Verts[i].Y } + FVector2D(tilePos)) + noisePos);
		Verts[i].Z = GetWaveHeight(FVector2D{ Verts[i].X, Verts[i].Y }, noiseStrength, noiseScale, noisePos, drift);
		//GetWaveHeight
	}
	ProceduralSeaTile->UpdateMeshSection_LinearColor(0, Verts, Norms, UVs, TArray<FLinearColor>(), TArray<FProcMeshTangent>());
}

float FUProceduralSeaTile::GetWaveHeight(const FVector2D& at, float noiseStrength, float noiseScale, const FVector2D& noisePos, const FVector2D& drift)
{
	FVector tilePos = ProceduralSeaTile->GetComponentTransform().GetTranslation();
	FVector2D p = (1 / noiseScale) * (at + FVector2D(tilePos)) + noisePos;
	const float sizeJitter = (FMath::PerlinNoise2D(p / 5.f) + 1.f) / 2.0f;
	const float phazeJitter = FMath::Abs(FMath::PerlinNoise2D(p / 75.f));
	const float phazeJitterSmall = FMath::Abs(FMath::PerlinNoise2D(p / 5.f));
	const float waveFuncParam = FVector2D::DotProduct(p, drift.GetSafeNormal()) + phazeJitter * 20.f + phazeJitterSmall * 3.f;
	const float sizeMultiplier = (0.5f * phazeJitter + 0.5f) * sizeJitter * noiseStrength;
	const float wavePower = FMath::Cos(waveFuncParam) * FMath::Sin(waveFuncParam) < 0. ? 0.5f : 2.f;
	const float wave = 1.f - FMath::Pow(FMath::Abs(FMath::Sin(waveFuncParam)), wavePower);
	return (wave - 0.5f) * 2.f * sizeMultiplier;
}
