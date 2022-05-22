// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerShip.h"

// Engine includes
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"

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
}

void APlayerShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraAttach->SetWorldLocation(FMath::VInterpTo(CameraAttach->GetComponentLocation(), this->GetActorLocation(), DeltaTime, 5.f));

	if (!bPhoenixReady) PhoenixTimer += DeltaTime;
	if (PhoenixTimer > 60.f)
	{
		bPhoenixReady = true;
	}

	ScanTimer += DeltaTime;
	if (ScanTimer > ScanFrequency)
	{
		SendArmorFuelToUI(CurrentShield / MaxShield, CurrentArmor / MaxArmor, CurrentFuel / MaxFuel, PhoenixTimer / 60.f);
		ScanTimer = 0.f;

		if (bIsDestroyed && bPhoenixReady)
		{
			SendMessageToUI(FText::FromString(TEXT("Phoenix systems activated. Ship has been reconstituted with minimal abilities.")));
			UpgradeShip(true);
			PhoenixTimer = 0.f;
			bPhoenixReady = false;
			bIsDestroyed = false;
		}
		else if (bIsDestroyed)
		{
			SendMessageToUI(FText::FromString(TEXT("Your ship was destroyed permanently.")));
			ShipPermanentDeath();
		}
		else
		{
			ScanForTargets();

			if (IsValid(ClosestTarget))
			{
				AJamShipBase* Ship = Cast<AJamShipBase>(ClosestTarget);
				if (IsValid(Ship))
				{
					TargetShip = Ship;
					ClosestTarget->ToggleArrows(true);
				}
			}
		}			
	}
}

void APlayerShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent) // If this fails, change back to the InputComponent variable
	{
		// Action bindings
		PlayerInputComponent->BindAction("SpeedBoost", IE_Pressed, this, &APlayerShip::InputStartSpeedBoost);
		PlayerInputComponent->BindAction("SpeedBoost", IE_Released, this, &APlayerShip::InputStopSpeedBoost);
		PlayerInputComponent->BindAction("TabTarget", IE_Pressed, this, &APlayerShip::SelectClosestTarget);
		PlayerInputComponent->BindAction("FireMissile", IE_Pressed, this, &APlayerShip::InputFireMissile);
		PlayerInputComponent->BindAction("LaunchFighters", IE_Pressed, this, &APlayerShip::InputLaunchFighters);

		PlayerInputComponent->BindAction("CameraZoomIn", IE_Pressed, this, &APlayerShip::InputCameraZoomIn);
		PlayerInputComponent->BindAction("CameraZoomOut", IE_Pressed, this, &APlayerShip::InputCameraZoomOut);
	}
	else { UE_LOG(LogTemp, Error, TEXT("Input component NOT found!")); }
}

void APlayerShip::ApplyLootableBonus(float InArmor, float InFuel)
{
	CurrentArmor += InArmor;
	CurrentArmor = FMath::Clamp(CurrentArmor, 0, MaxArmor);

	CurrentFuel += InFuel;
	CurrentFuel = FMath::Clamp(CurrentFuel, 0, MaxFuel);
}

void APlayerShip::UpgradeShip(bool IsTierOneReset)
{
	if (IsTierOneReset) UpgradeLevel = 1;
	else ++UpgradeLevel;
	UpgradeLevel = FMath::Clamp(UpgradeLevel, 1, 4);

	switch (UpgradeLevel)
	{
	case 1: break;
	case 2: break;
	case 3: break;
	case 4: bShieldEnabled = true; break;
	}

	CurrentShield = MaxShield;
	CurrentArmor = MaxArmor;
	CurrentFuel = MaxFuel;

	//SetupTurrets();
	// Set static mesh
	// Update physics mass
	// Update turret amounts -> turret damage
}

void APlayerShip::InputCameraZoomIn()
{
	if (TopDownCamera->GetRelativeLocation().X < -2000) TopDownCamera->AddLocalOffset(FVector(500.f, 0.f, 0.f));
}

void APlayerShip::InputCameraZoomOut()
{
	if (TopDownCamera->GetRelativeLocation().X > -10000) TopDownCamera->AddLocalOffset(FVector(-500.f, 0.f, 0.f));
}

void APlayerShip::InputStartSpeedBoost()
{
	bIsBoosting = true;
}

void APlayerShip::InputStopSpeedBoost()
{
	bIsBoosting = false;
}

void APlayerShip::InputFireMissile()
{
	SendMessageToUI(FText::FromString(TEXT("Launching a missile at current target.")));
}

void APlayerShip::InputLaunchFighters()
{
	SendMessageToUI(FText::FromString(TEXT("Deploying fighters to defend the ship!")));
}

void APlayerShip::ScanForTargets()
{
	AllTargets.Empty();

	TArray<AActor*> OverlappingActors;
	TargetField->GetOverlappingActors(OverlappingActors, TSubclassOf<AActor>());
	for (AActor* ShipActor : OverlappingActors)
	{
		ANPCShip* NPCShip = Cast<ANPCShip>(ShipActor);
		if (IsValid(NPCShip)) AllTargets.Add(NPCShip);
	}
	if (!IsValid(ClosestTarget)) SelectClosestTarget();
}

void APlayerShip::SelectClosestTarget()
{
	if (IsValid(ClosestTarget)) ClosestTarget->ToggleArrows(false);

	float ShortestDistance = 50000.f;
	for (ANPCShip* NPCShip : AllTargets)
	{
		if (IsValid(NPCShip))
		{
			NPCShip->ToggleArrows(false);
			float Distance = FVector::Distance(this->GetActorLocation(), NPCShip->GetActorLocation());
			if (Distance < ShortestDistance)
			{
				ShortestDistance = Distance;
				ClosestTarget = NPCShip;
			}
		}
	}
	if (IsValid(ClosestTarget)) ClosestTarget->ToggleArrows(true);
}

void APlayerShip::ManualSelectTarget(ANPCShip* InNewTarget)
{
	if (IsValid(ClosestTarget)) ClosestTarget->ToggleArrows(false);
	ClosestTarget = InNewTarget;
	if (IsValid(ClosestTarget)) ClosestTarget->ToggleArrows(true);
}