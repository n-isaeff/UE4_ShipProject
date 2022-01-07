// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawn.h"
//#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "ProceduralSeaActor.h"


// Sets default values
AShipPawn::AShipPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ShipRoot = CreateDefaultSubobject<USceneComponent>("ShipRoot");
	SetRootComponent(ShipRoot);
	SpringArmTarget = CreateDefaultSubobject<USceneComponent>("SpringArmTarget");
	SpringArmTarget->SetupAttachment(ShipRoot);
	ShipHull = CreateDefaultSubobject<UStaticMeshComponent>("ShipHull");
	ShipHull->SetupAttachment(ShipRoot);
	ShipRudder = CreateDefaultSubobject<UStaticMeshComponent>("ShipRudder");
	ShipRudder->SetupAttachment(ShipHull);

	m_bNames = {
		"Front", "Back", "Left", "Right"
	};

	for (int i = 0; i < m_sailsNum; i++)
	{
		FString name = "ShipSail_" + FString::FromInt(i + 1);
		ShipSails.Add(CreateDefaultSubobject<UStaticMeshComponent>(FName(name)));
		ShipSails[i]->SetupAttachment(ShipHull);
	}
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(SpringArmTarget);

	for (int i = 0; i < 4; i++)
	{
		FString name = "BuoyancyPoint" + m_bNames[i];
		BuoyancyPoints.Add(m_bNames[i], CreateDefaultSubobject<USceneComponent>(FName(name)));
		BuoyancyPoints[m_bNames[i]]->SetupAttachment(ShipHull);
		BuoyancyPointsWaterline.Add(m_bNames[i], 0);
	}

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();
	if (ShipPlayer)
	{
		Tags.Add(FName("PlayerShip"));
	}
	FVector hullMin, hullMax;
	ShipHull->GetLocalBounds(hullMin, hullMax);
	m_hullSideA = (hullMax.X - hullMin.X);
	m_hullSideB = (hullMax.Y - hullMin.Y);
}

FVector2D AShipPawn::GetWindForce() const
{
	//FVector2D va = GetWindSpeed() - m_horSpeed;
	FRotator sailRot(0, SailAngle, 0);
	FVector forward = ShipHull->GetForwardVector();
	FVector2D sails(sailRot.RotateVector(forward));
	float effect = FVector2D::DotProduct(WindSpeed, sails);
	float sailsEffect = FVector2D::DotProduct(FVector2D(forward), sails);
	return effect > 0.001f ? FVector2D(forward * effect * sailsEffect) : FVector2D(0);
}

FVector2D AShipPawn::GetFlowForce() const
{
	//FVector2D fa = GetFlowSpeed() - m_horSpeed;
	return FlowSpeed;
}

float AShipPawn::WaterLevelPerPoint(const FVector& point, bool& found, const float searchRange)
{
	if (!m_seaInstance)
	{
		TArray<AActor*> levelSea;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("CurrentSea"), levelSea);
		if (!levelSea.Num()) return 0;
		m_seaInstance = (AProceduralSeaActor*)levelSea[0];
	}
	FHitResult hit;
	FCollisionQueryParams collisionParams;
	collisionParams.bTraceComplex = true;
	const FVector searchRangeVec = FVector(0, 0, searchRange);
	if (m_seaInstance->ActorLineTraceSingle(hit, point + searchRangeVec, point - searchRangeVec, ECC_WorldStatic, collisionParams))
	{
		found = true;
		return -hit.Distance + searchRange;
	}
	found = false;
	return 0;
}

float AShipPawn::CalculateWaterlineAvr()
{
	float avr = 0;
	int foundNum = 0;
	for (int i = 0; i < m_bNames.Num(); i++)
	{
		FString& name = m_bNames[i];
		bool hitFound = false;
		BuoyancyPointsWaterline[name] = WaterLevelPerPoint(BuoyancyPoints[name]->GetComponentTransform().GetLocation(), hitFound);
		if (hitFound)
		{
			avr += BuoyancyPointsWaterline[name];
			foundNum++;
		}
	}
	if (foundNum > 0)
	{
		return avr / foundNum;
	}
	FVector hullMin, hullMax;
	ShipHull->GetLocalBounds(hullMin, hullMax);
	return - BuoyancyPoints[m_bNames[0]]->GetRelativeLocation().Z + hullMin.Z;
	
}

void AShipPawn::CalculateBuoyantForces()
{
	FVector hullMin, hullMax;
	ShipHull->GetLocalBounds(hullMin, hullMax);
	for (int i = 0; i < m_bNames.Num(); i++)
	{
		FString& name = m_bNames[i];
		bool hitFound = false;
		float hitDist = WaterLevelPerPoint(BuoyancyPoints[name]->GetComponentTransform().GetLocation(), hitFound);
		if (hitFound)
		{
			if (hitDist < 0)
			{
				FVector sampleLoc = BuoyancyPoints[name]->GetRelativeLocation();
				ShipHull->AddForceAtLocationLocal(FVector(0, 0, -hitDist * 1000), sampleLoc);
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, name + FString::Printf(TEXT(" F:%f"), BuoyancyPointsWaterline[name]));
				}
			}
		}
		BuoyancyPointsWaterline[name] = hitDist;
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("DONE"));
	}
}


void AShipPawn::RotateCamera()
{
	auto camRot = GetActorRotation();
	camRot.Pitch += m_cameraOrient.Y;
	camRot.Yaw += m_cameraOrient.X;
	SpringArm->SetRelativeRotation(camRot);
}

// Called every frame
void AShipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float avr = FMath::Clamp(CalculateWaterlineAvr(), 0.f, 50.f);
	for (const auto& p : BuoyancyPoints)
	{
		const FString& bName = p.Key;
		FVector forceLoc = p.Value->GetComponentLocation();
		float waterline = FMath::Clamp(BuoyancyPointsWaterline[bName], 0.f, 50.f);
		ShipHull->AddForceAtLocation(FVector(0, 0, 1) * waterline * 20000, forceLoc);
	}
	if (FMath::Abs(RudderAngle) > 0.001f && BuoyancyPointsWaterline["Back"] > 0.f)
	{
		//FRotator r(0, RudderAngle * 0.01f, 0);
		FVector currentSpeed = ShipHull->GetComponentVelocity();
		FVector2D curHorSpeed(currentSpeed);
		ShipHull->AddTorque({ 0, 0, -100.0f * RudderAngle * curHorSpeed.Size() });
	}
	ShipHull->AddForce(FVector(GetWindForce(), 0.f));
	ShipHull->AddForce(FVector(GetFlowForce(), 0.f));
	RotateCamera();
	SpringArmTarget->SetWorldLocation(FVector(FVector2D(ShipHull->GetComponentLocation()), 0));
}

// Called to bind functionality to input
void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("TurnRudder", this, &AShipPawn::TurnRudder);
	PlayerInputComponent->BindAxis("TurnSail", this, &AShipPawn::TurnSail);

	PlayerInputComponent->BindAxis("CameraYaw", this, &AShipPawn::CameraYaw);
	PlayerInputComponent->BindAxis("CameraPitch", this, &AShipPawn::CameraPitch);
}

void AShipPawn::TurnRudder(float val)
{
	const auto oldRudder = RudderAngle;
	RudderAngle = FMath::Clamp(RudderAngle + val, -90.0f, 90.0f);
	TurnShipPart(ShipRudder, RudderAngle - oldRudder);
}

void AShipPawn::TurnSail(float val)
{
	auto oldAngle = SailAngle;
	SailAngle = FMath::Clamp(SailAngle + val, -90.0f, 90.0f);
	for (int i = 0; i < m_sailsNum; i++)
	{
		TurnShipPart(ShipSails[i], SailAngle - oldAngle);
	}
}

void AShipPawn::CameraPitch(float val)
{
	m_cameraOrient.Y += val;
	m_cameraOrient.Y = FMath::Clamp(m_cameraOrient.Y, -85.f, 0.f);
}

void AShipPawn::CameraYaw(float val)
{
	m_cameraOrient.X += val;
}

void AShipPawn::CameraZoom(float val)
{

}

void AShipPawn::TurnShipPart(UStaticMeshComponent* part, float degrees)
{
	FVector up = part->GetRelativeRotation().RotateVector({ 0, 0, 1 });
	FQuat r(up, FMath::DegreesToRadians(degrees));
	part->AddRelativeRotation(r);
}

