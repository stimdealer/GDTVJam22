// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerShip.h"

// Engine includes
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

// Local includes
#include "NPCShip.h"

APlayerShip::APlayerShip()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraAttach = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Attach"));

	CameraAzimuth = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Azimuth"));
	CameraAzimuth->AttachToComponent(CameraAttach, FAttachmentTransformRules::KeepRelativeTransform);
	
	CameraElevation = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Elevation"));
	CameraElevation->AttachToComponent(CameraAzimuth, FAttachmentTransformRules::KeepRelativeTransform);
	CameraElevation->AddLocalRotation(FRotator(-60.0, 0, 0));

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Top Down Camera"));
	TopDownCamera->AttachToComponent(CameraElevation, FAttachmentTransformRules::KeepRelativeTransform);
	TopDownCamera->AddLocalOffset(FVector(-5000.0, 0, 0));

	TargetField = CreateDefaultSubobject<USphereComponent>(TEXT("Target Field"));
	TargetField->AttachToComponent(PhysicsRoot, FAttachmentTransformRules::KeepRelativeTransform);
	TargetField->SetSphereRadius(6000.f);
	TargetField->bHiddenInGame = false;

	// Initial stats for Tier 1 ship:
	MaxShield = 0.f;
	CurrentShield = 0.f;
	MaxArmor = 1000.f;
	CurrentArmor = 1000.f;
	MaxFuel = 100.f;
	CurrentFuel = 100.f;
	MaxSpeed = 700.f;
	TurnSpeed = 80.f;
	bShieldEnabled = false;
	bBroadsides = false;
	bLauncher = false;
	bFighters = false;

	TurretsFirepower = 10.f;
	TurretRange = 5000.f;
}

void APlayerShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraAttach->SetWorldLocation(FMath::VInterpTo(CameraAttach->GetComponentLocation(), CameraLead, DeltaTime, 3.f));
	UpdateQuestMarkers();

	if (bIsDestroyed && bPhoenixReady)
	{
		SendMessageToUI(FText::FromString(TEXT("Phoenix systems activated. Ship has been reconstituted with minimal abilities.")));
		UpgradeShip(true);
		PhoenixTimer = 0.f;
		bPhoenixReady = false;
		bIsDestroyed = false;
	}
	else if (bIsDestroyed && !bDestructionComplete)
	{
		bDestructionComplete = true;
		SendMessageToUI(FText::FromString(TEXT("Your ship was destroyed permanently.")));
		ShipPermanentDeath();
	}

	if (!bPhoenixReady) PhoenixTimer += DeltaTime;
	if (PhoenixTimer > 60.f)
	{
		bPhoenixReady = true;
	}

	ScanTimer += DeltaTime;
	if (ScanTimer > ScanFrequency)
	{
		SendArmorFuelToUI(
			CalculatePercent(CurrentShield, MaxShield),
			CalculatePercent(CurrentArmor, MaxArmor),
			CalculatePercent(CurrentFuel, MaxFuel),
			CalculatePercent(CurrentOre, MaxOre),
			CalculatePercent(PhoenixTimer, 60.f)
		);
		SendIconsToUI(bBroadsides, bLauncher, bFighters, bMissileReady);
		ScanTimer = 0.f;

		UpdateFighters();

		FighterCount = MaxFighters - DeployedFighters.Num();
		FighterCount = FMath::Clamp(FighterCount, 0, MaxFighters);

		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::FromInt(FighterCount));

		ScanForTargets();

		if (IsValid(ClosestNPCShipTarget))
		{
			AJamShipBase* Ship = Cast<AJamShipBase>(ClosestNPCShipTarget);
			if (IsValid(Ship))
			{
				TurretTargetShip = Ship;
				ClosestNPCShipTarget->ToggleTurretArrows(true);
			}
		}	
	}
}

void APlayerShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		// Action bindings
		PlayerInputComponent->BindAction("SpeedBoost", IE_Pressed, this, &APlayerShip::InputStartSpeedBoost);
		PlayerInputComponent->BindAction("SpeedBoost", IE_Released, this, &APlayerShip::InputStopSpeedBoost);
		PlayerInputComponent->BindAction("TabTarget", IE_Pressed, this, &APlayerShip::SelectClosestTarget);

		PlayerInputComponent->BindAction("CameraZoomIn", IE_Pressed, this, &APlayerShip::InputCameraZoomIn);
		PlayerInputComponent->BindAction("CameraZoomOut", IE_Pressed, this, &APlayerShip::InputCameraZoomOut);
	}
	else { UE_LOG(LogTemp, Error, TEXT("Input component NOT found!")); }
}

void APlayerShip::ApplyLootableBonus(int32 InType, int32 InAmount)
{
	switch (InType)
	{
	case 0:
		CurrentShield += InAmount;
		CurrentShield = FMath::Clamp(CurrentShield, 0, MaxShield);
		break;
	case 1:
		CurrentArmor += InAmount;
		CurrentArmor = FMath::Clamp(CurrentArmor, 0, MaxArmor);
		break;
	case 2:
		CurrentFuel += InAmount;
		CurrentFuel = FMath::Clamp(CurrentFuel, 0, MaxFuel);
		break;
	case 3:
		CurrentOre += InAmount;
		int32 Overflow = CurrentOre - MaxOre;
		CurrentOre = FMath::Clamp(CurrentOre, 0, MaxOre);
		if (CurrentOre == MaxOre) UpgradeShip(Overflow);
		break;
	}
}

void APlayerShip::UpgradeShip(int32 InOreOverflow, bool IsTierOneReset)
{
	if (IsTierOneReset)
	{
		UpgradeLevel = 1;
		PhoenixExplosion();
		PhysicsRoot->SetPhysicsLinearVelocity(FVector(0.0));
	}
	else if (UpgradeLevel == 4) return;
	else ++UpgradeLevel;
	UpgradeLevel = FMath::Clamp(UpgradeLevel, 1, 4);

	switch (UpgradeLevel)
	{
	case 1:
		TurretsFirepower = 10.f;
		BroadsidesFirepower = 0.f;
		TurretRange = 5000.f;

		MaxShield = 0.f;
		MaxArmor = 1000.f;
		MaxFuel = 100.f;
		CurrentFuel = 100.f;

		MaxSpeed = 700.f;
		TurnSpeed = 80.f;
		MaxOre = 100;

		bShieldEnabled = false;
		bBroadsides = false;
		bLauncher = false;
		bFighters = false;

		break;
	case 2:
		TurretsFirepower = 15.f;
		BroadsidesFirepower = 30.f;
		BroadsideRange = 5000.f;
		TurretRange = 6000.f;

		MaxShield = 0.f;
		MaxArmor = 1500.f;
		MaxFuel = 100.f;
		CurrentFuel = 100.f;

		MaxSpeed = 850.f;
		TurnSpeed = 90.f;
		MaxOre = 150;

		bShieldEnabled = false;
		bBroadsides = true;
		bLauncher = false;
		bFighters = false;

		break;
	case 3:
		TurretsFirepower = 20.f;
		BroadsidesFirepower = 50.f;
		BroadsideRange = 5500.f;
		TurretRange = 7000.f;

		MaxShield = 500.f;
		MaxArmor = 1500.f;
		MaxFuel = 100.f;
		CurrentFuel = 100.f;

		MaxSpeed = 1000.f;
		TurnSpeed = 100.f;
		MaxOre = 200;

		bShieldEnabled = true;
		bBroadsides = true;
		bLauncher = true;
		bFighters = false;

		break;
	case 4:
		TurretsFirepower = 25.f;
		BroadsidesFirepower = 70.f;
		BroadsideRange = 6000.f;
		TurretRange = 8000.f;

		MaxShield = 1000.f;
		MaxArmor = 2000.f;
		MaxFuel = 100.f;
		CurrentFuel = 100.f;

		MaxSpeed = 1200.f;
		TurnSpeed = 120.f;
		MaxOre = 0;

		bShieldEnabled = true;
		bBroadsides = true;
		bLauncher = true;
		bFighters = true;

		break;
	}

	CurrentShield = MaxShield;
	CurrentArmor = MaxArmor;
	CurrentFuel = MaxFuel;
	CurrentOre = InOreOverflow;

	SendUpgradeLevelToUI(UpgradeLevel);
}

void APlayerShip::AddQuest(const FString& InQuestName, FVector InLocation, int32 InIconType)
{
	FQuestMarker NewQuestMarker;
	NewQuestMarker.MarkerTitle = InQuestName;
	NewQuestMarker.Location = InLocation;
	NewQuestMarker.bReached = false;
	NewQuestMarker.bCompleted = false;
	NewQuestMarker.IconType = InIconType;
	ActiveQuestMarkers.Add(NewQuestMarker);
	FString NewQuestString = FString(TEXT("New objective added: ") + InQuestName);
	SendMessageToUI(FText::FromString(NewQuestString));
}

void APlayerShip::UpdateQuest(const FString& InQuestName, bool InCompleted)
{
	for (FQuestMarker& Marker : ActiveQuestMarkers)
	{
		if (Marker.MarkerTitle == InQuestName) Marker.bCompleted = InCompleted;
	}
}

void APlayerShip::UpdateQuestMarkers()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (IsValid(PC))
	{
		FVector2D ScreenPosition;

		for (const FQuestMarker& Marker : ActiveQuestMarkers)
		{
			bool bEnlargeIcon = FVector::Distance(this->GetActorLocation(), Marker.Location) < 25000.f;

			if (PC->ProjectWorldLocationToScreen(Marker.Location, ScreenPosition, true))
			{
				UpdateQuestMarkerUI(Marker, ScreenPosition, bEnlargeIcon);
			}
		}
	}
}

void APlayerShip::InputCameraZoomIn()
{
	if (TopDownCamera->GetRelativeLocation().X < -4000) TopDownCamera->AddLocalOffset(FVector(500.f, 0.f, 0.f));
}

void APlayerShip::InputCameraZoomOut()
{
	if (TopDownCamera->GetRelativeLocation().X > -12000) TopDownCamera->AddLocalOffset(FVector(-500.f, 0.f, 0.f));
}

void APlayerShip::InputStartSpeedBoost()
{
	bIsBoosting = true;
}

void APlayerShip::InputStopSpeedBoost()
{
	bIsBoosting = false;
}

void APlayerShip::ScanForTargets()
{
	AllTargets.Empty();

	TArray<AActor*> OverlappingActors;
	TargetField->GetOverlappingActors(OverlappingActors, TSubclassOf<ANPCShip>());

	for (AActor* ShipActor : OverlappingActors)
	{
		ANPCShip* NPCShip = Cast<ANPCShip>(ShipActor);
		if (IsValid(NPCShip)) AllTargets.Add(NPCShip);
	}

	SelectClosestTarget();
}

void APlayerShip::SelectClosestTarget()
{
	if (IsValid(ClosestNPCShipTarget))
	{
		if (!bManualTargetSelected) ClosestNPCShipTarget->ToggleTurretArrows(false);
		ClosestNPCShipTarget->ToggleBroadsideArrows(false);
	}		

	ANPCShip* NewClosestNPCShip = nullptr;
	float ShortestDistance = 100000.f;
	for (AJamShipBase* Ship : AllTargets)
	{
		ANPCShip* NPCShip = Cast<ANPCShip>(Ship);
		if (IsValid(NPCShip))
		{
			float Distance = FVector::Distance(this->GetActorLocation(), NPCShip->GetActorLocation());
			if (Distance < ShortestDistance)
			{
				ShortestDistance = Distance;
				NewClosestNPCShip = NPCShip;
			}
		}
	}

	if (!IsValid(ClosestNPCShipTarget) && !bManualTargetSelected) ClosestNPCShipTarget = NewClosestNPCShip;
	else ClosestNPCShipTarget->ToggleTurretArrows(true);
	
	if (bBroadsides) BroadsideTargetShip = NewClosestNPCShip;
	if (IsValid(NewClosestNPCShip)) NewClosestNPCShip->ToggleBroadsideArrows(true);

	if (IsValid(NewClosestNPCShip) && bFighters && FighterCount >= 1)
	{
		LaunchFighter(NewClosestNPCShip);
	}
}

void APlayerShip::ManualSelectTarget(AJamShipBase* InNewTarget)
{
	if (IsValid(ClosestNPCShipTarget)) ClosestNPCShipTarget->ToggleTurretArrows(false);

	auto NewNPCTarget = Cast<ANPCShip>(InNewTarget);
	if (IsValid(NewNPCTarget)) ClosestNPCShipTarget = NewNPCTarget;
	if (IsValid(ClosestNPCShipTarget)) ClosestNPCShipTarget->ToggleTurretArrows(true);

	if (IsValid(ClosestNPCShipTarget) && bFighters && FighterCount >= 1)
	{
		LaunchFighter(ClosestNPCShipTarget);
	}
}

float APlayerShip::CalculatePercent(float InCurrent, float InMax)
{
	if (FMath::IsNearlyZero(InCurrent, 0.001)) return 0.f;
	else return InCurrent / InMax;
}

void APlayerShip::UpdateFighters()
{
	TArray<AJamShipBase*> FightersToCheck = DeployedFighters;
	for (int32 i = 0; i < FightersToCheck.Num(); ++i)
	{
		if (!IsValid(FightersToCheck[i])) DeployedFighters.Remove(FightersToCheck[i]);
	}
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString::FromInt(DeployedFighters.Num()));
}

bool APlayerShip::GetFightersStatus() {	return bFighters; }