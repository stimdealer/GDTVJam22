// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerShip.h"

// Engine includes
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"

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
}

void APlayerShip::Tick(float DeltaTime)
{
	MoveToDestination(DeltaTime);
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

void APlayerShip::MoveToDestination(float InDelta)
{
	if (!IsValid(PhysicsRoot)) return;

	CameraAttach->SetWorldLocation(FMath::VInterpTo(CameraAttach->GetComponentLocation(), this->GetActorLocation(), InDelta, 5.f));

	FVector Heading = FVector(Destination - this->GetActorLocation()).GetSafeNormal();

	float Distance = FVector::Distance(this->GetActorLocation(), Destination);
	float AngleDistance = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorForwardVector(), Heading)));

	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::SanitizeFloat(Distance));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Purple, FString::SanitizeFloat(AngleDistance));

	UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(PhysicsRoot);
	if (IsValid(Prim))
	{
		// Apply rotation
		if (Distance > 1000.f) Prim->SetWorldRotation(FMath::RInterpTo(Prim->GetComponentRotation(), Heading.Rotation(), InDelta, 1.f));

		float Momentum;
		FVector Direction;
		Prim->GetComponentVelocity().ToDirectionAndLength(Direction, Momentum);

		// Apply forward thrust
		FVector Force = FVector(0.0);
		if (Distance > 400.0 && Momentum < MaxSpeed * 1.25 && AngleDistance < 10.0)
		{
			if (Distance > 1000.0) Thrust = 50000.f;
			else if (Distance > 600.0) Thrust = 20000.f;
			else Thrust = 10000.f;

			Force = Prim->GetForwardVector() * Thrust * InDelta;
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, TEXT("Stopping Ship"));
			if (Prim->GetComponentVelocity().X > 5.0 || Prim->GetComponentVelocity().Y > 5.0 || Prim->GetComponentVelocity().Z > 5.0)
			{
				Force = Prim->GetComponentVelocity() * -5;
			}
		}

		Force = FVector(
			FMath::Clamp(Force.X, -MaxSpeed, MaxSpeed),
			FMath::Clamp(Force.Y, -MaxSpeed, MaxSpeed),
			FMath::Clamp(Force.Z, -MaxSpeed, MaxSpeed)
		);

		Prim->AddForce(Force, NAME_None, true);
	}
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
}
