// Fill out your copyright notice in the Description page of Project Settings.


#include "JamShipBase.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine/Public/DrawDebugHelpers.h"

// Sets default values
AJamShipBase::AJamShipBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PhysicsRoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Physics Root"));
	PhysicsRoot->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));
	this->RootComponent = PhysicsRoot;
	PhysicsRoot->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void AJamShipBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJamShipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveToDestination(DeltaTime);
	TurretsTracking(DeltaTime);

	if (bIsBoosting) CurrentFuel -= DeltaTime;

	if (IsValid(TargetShip))
	{
		WeaponsTimer += DeltaTime;
		if (WeaponsTimer > 0.5f)
		{
			WeaponsTimer = 0.f;
			FireWeapons();
		}
	}
}

void AJamShipBase::FireWeapons()
{
	if (bIsTurretsInRange && bIsTurretsAimedAtTarget) TargetShip->ApplyDamage(TurretsFirepower);
}

void AJamShipBase::SetupTurret(FName InSocket, ETurretType InType)
{
	if (PhysicsRoot->GetStaticMesh()->FindSocket(InSocket))
	{
		UStaticMeshComponent* NewTurret = NewObject<UStaticMeshComponent>(this);
		NewTurret->RegisterComponent();
		NewTurret->AttachToComponent(PhysicsRoot, FAttachmentTransformRules::KeepRelativeTransform, InSocket);
		Turrets.Add(NewTurret);
	}
}

void AJamShipBase::MoveToDestination(float InDelta)
{
	if (!IsValid(PhysicsRoot)) return;

	FVector Heading = FVector(Destination - this->GetActorLocation()).GetSafeNormal();

	float Distance = FVector::Distance(this->GetActorLocation(), Destination);
	float AngleDistance = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorForwardVector(), Heading)));

	UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(PhysicsRoot);
	if (IsValid(Prim))
	{
		// Apply rotation
		if (Distance > 1000.f) Prim->SetWorldRotation(FMath::RInterpTo(Prim->GetComponentRotation(), Heading.Rotation(), InDelta, float(TurnSpeed / Prim->GetMass())));

		float Momentum;
		FVector Direction;
		Prim->GetComponentVelocity().ToDirectionAndLength(Direction, Momentum);

		// Apply forward thrust
		FVector Force = FVector(0.0);
		float NewMaxSpeed;
		if (bIsBoosting && CurrentFuel > 0.f) NewMaxSpeed = MaxSpeed * 2.f;
		else NewMaxSpeed = MaxSpeed;

		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, FString::SanitizeFloat(NewMaxSpeed));

		if (Distance > 500.0 && Momentum < NewMaxSpeed * 1.25 && AngleDistance < 45.0)
		{
			if (bIsBoosting && Distance > 1000.0) Thrust = 100000.f;
			else if (Distance > 1000.0) Thrust = 50000.f;
			else Thrust = 25000.f;

			Force = Prim->GetForwardVector() * Thrust * InDelta;
		}
		else
		{
			if (Prim->GetComponentVelocity().X > 5.0 || Prim->GetComponentVelocity().Y > 5.0 || Prim->GetComponentVelocity().Z > 5.0)
			{
				Force = Prim->GetComponentVelocity() * -2;
			}
		}

		Force = FVector(
			FMath::Clamp(Force.X, -NewMaxSpeed, NewMaxSpeed),
			FMath::Clamp(Force.Y, -NewMaxSpeed, NewMaxSpeed),
			FMath::Clamp(Force.Z, -NewMaxSpeed, NewMaxSpeed)
		);

		Prim->AddForce(Force, NAME_None, true);
	}
}

void AJamShipBase::TurretsTracking(float InDelta)
{
	double NewYaw = 0.0;
	for (UStaticMeshComponent* Turret : Turrets)
	{
		if (Turret->GetStaticMesh()->FindSocket(TEXT("Fire")))
		{
			if (IsValid(TargetShip))
			{
				FVector TargetLocation = TargetShip->GetActorLocation();
				FTransform TurretTransform = this->GetActorTransform();
				FVector Translate = TargetLocation - Turret->GetComponentLocation();
				FVector Inverse = UKismetMathLibrary::InverseTransformDirection(TurretTransform, Translate);
				FRotator RotFromX = UKismetMathLibrary::MakeRotFromX(Inverse);
				NewYaw = RotFromX.Yaw;

				float TurretAngleToTarget = FMath::RadiansToDegrees(acosf(FVector::DotProduct(Turret->GetForwardVector(), Translate.GetSafeNormal())));
				bIsTurretsAimedAtTarget = TurretAngleToTarget < 30.f;

				float TargetDistance = FVector::Distance(TargetShip->GetActorLocation(), this->GetActorLocation());
				bIsTurretsInRange = TargetDistance < WeaponsRange;
				GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::SanitizeFloat(TargetDistance));

				// Debug lines for testing
				if (bIsTurretsAimedAtTarget && bIsTurretsInRange) DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Green, false, 0.1f, 0.f, 10.f);
				else DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Red, false, 0.1f, 0.f, 10.f);
			}

			FRotator NewRotation = FMath::RInterpConstantTo(Turret->GetRelativeRotation(), FRotator(0.0, NewYaw, 0.0), InDelta, 100.f);
			Turret->SetRelativeRotation(NewRotation);
		}
	}
}

void AJamShipBase::ApplyDamage(float InDamage)
{
	CurrentHealth -= InDamage;
	if (CurrentHealth <= 0)
	{
		bIsDestroyed = true;
	}
}

float AJamShipBase::GetHealthPercent()
{
	return CurrentHealth / MaxHealth;
}

float AJamShipBase::GetFuelPercent()
{
	return CurrentFuel / MaxFuel;
}
