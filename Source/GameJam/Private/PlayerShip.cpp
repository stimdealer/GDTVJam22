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
	//CameraAttach->AttachToComponent(PhysicsRoot, FAttachmentTransformRules::KeepRelativeTransform);

	CameraAzimuth = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Azimuth"));
	CameraAzimuth->AttachToComponent(CameraAttach, FAttachmentTransformRules::KeepRelativeTransform);
	
	CameraElevation = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Elevation"));
	CameraElevation->AttachToComponent(CameraAzimuth, FAttachmentTransformRules::KeepRelativeTransform);
	CameraElevation->AddLocalRotation(FRotator(-60.0, 0, 0));

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Top Down Camera"));
	TopDownCamera->AttachToComponent(CameraElevation, FAttachmentTransformRules::KeepRelativeTransform);
	TopDownCamera->AddLocalOffset(FVector(-5000.0, 0, 0));

	TargetField = CreateDefaultSubobject<USphereComponent>(TEXT("Target Field"));
}

void APlayerShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ScanTimer += DeltaTime;
	if (ScanTimer > ScanFrequency)
	{
		ScanTimer = 0.f;
		ScanForTargets();
	}

	CameraAttach->SetWorldLocation(FMath::VInterpTo(CameraAttach->GetComponentLocation(), this->GetActorLocation(), DeltaTime, 5.f));


}

void APlayerShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent) // If this fails, change back to the InputComponent variable
	{
		// Action bindings
		PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &APlayerShip::StartLeftClick);
		PlayerInputComponent->BindAction("PrimaryAction", IE_Released, this, &APlayerShip::StopLeftClick);
		PlayerInputComponent->BindAction("SecondaryAction", IE_Pressed, this, &APlayerShip::StartRightClick);
		PlayerInputComponent->BindAction("SecondaryAction", IE_Released, this, &APlayerShip::StopRightClick);

		// Axis bindings
		PlayerInputComponent->BindAxis("NavMouseX", this, &APlayerShip::InputNavMouseX);
		PlayerInputComponent->BindAxis("NavMouseY", this, &APlayerShip::InputNavMouseY);
		PlayerInputComponent->BindAxis("CameraZoom", this, &APlayerShip::InputCameraZoom);
	}
	else { UE_LOG(LogTemp, Error, TEXT("Input component NOT found!")); }
}

void APlayerShip::StartLeftClick()
{
	bPrimaryPressed = true;
}

void APlayerShip::StopLeftClick()
{
	bPrimaryPressed = false;
}

void APlayerShip::StartRightClick()
{
}

void APlayerShip::StopRightClick()
{
}

void APlayerShip::InputNavMouseX(float Value)
{
}

void APlayerShip::InputNavMouseY(float Value)
{
}

void APlayerShip::InputCameraZoom(float Value)
{
	TopDownCamera->AddLocalOffset(FVector(Value * 500.f, 0.f, 0.f));
}

TArray<ANPCShip*> APlayerShip::ScanForTargets()
{
	TArray<AActor*> OverlappingActors;
	TargetField->GetOverlappingActors(OverlappingActors, TSubclassOf<AJamShipBase>());

	TArray<ANPCShip*> NPCShips;
	for (AActor* ShipActor : OverlappingActors)
	{
		ANPCShip* NPCShip = Cast<ANPCShip>(ShipActor);
		if (IsValid(NPCShip)) NPCShips.Add(NPCShip);
	}

	GEngine->AddOnScreenDebugMessage(-1, 0.4f, FColor::Cyan, FString::SanitizeFloat(NPCShips.Num()));

	return NPCShips;
}
