// Fill out your copyright notice in the Description page of Project Settings.


#include "JamShipBase.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

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

// Called every frame
void AJamShipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveToDestination(DeltaTime);
	TurretsTracking(DeltaTime);
	BroadsidesTracking(TargetShip);

	if (bIsBoosting) CurrentFuel -= DeltaTime;

	if (bShieldDown) ShieldRegenDelay += DeltaTime;
	else ShieldRegenTimer += DeltaTime;

	if (ShieldRegenDelay > 10.f)
	{
		ShieldRegenDelay = 0.f;
		bShieldDown = false;
	}
	if (ShieldRegenTimer > 2.f)
	{
		ShieldRegenTimer = 0.f;
		CurrentShield += MaxShield * 0.1f;
		CurrentShield = FMath::Clamp(CurrentShield, 0, MaxShield);
	}

	if (IsValid(TargetShip))
	{
		WeaponsTimer += DeltaTime;
		if (WeaponsTimer > 0.5f)
		{
			WeaponsTimer = 0.f;
			FireTurrets();
		}
	}
}

void AJamShipBase::FireTurrets()
{
	if (bIsTurretsInRange && bIsTurretsAimedAtTarget) TargetShip->ShipApplyDamage(TurretsFirepower);
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
		if (Distance > 500.f) Prim->SetWorldRotation(FMath::RInterpTo(Prim->GetComponentRotation(), Heading.Rotation(), InDelta, float(TurnSpeed / Prim->GetMass())));

		float Momentum;
		FVector Direction;
		Prim->GetComponentVelocity().ToDirectionAndLength(Direction, Momentum);

		//float AngleToDestination = FMath::RadiansToDegrees(acosf(FVector::DotProduct(Direction, Heading)));
		//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, *FString(TEXT("Velocity to destination angle (drift) difference:") + FString::SanitizeFloat(AngleToDestination)));

		// Apply forward thrust
		FVector Force = FVector(0.0);
		float NewMaxSpeed;
		if (bIsBoosting && CurrentFuel > 0.f) NewMaxSpeed = MaxSpeed * 2.f;
		else NewMaxSpeed = MaxSpeed;

		float Thrust = MaxSpeed * 100.f;

		if (Distance > 500.0 && Momentum < NewMaxSpeed)
		{
			if (bIsBoosting) Thrust = Thrust * 2.f;
			//else if (Distance < 1000.0) Thrust = 50000.f;

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
		if (IsValid(Turret) && Turret->GetStaticMesh()->FindSocket(TEXT("Fire")))
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

				// Debug lines for testing
				//if (bIsTurretsAimedAtTarget && bIsTurretsInRange) DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Green, false, 0.1f, 0.f, 10.f);
				//else DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Red, false, 0.1f, 0.f, 10.f);
			}

			FRotator NewRotation = FMath::RInterpConstantTo(Turret->GetRelativeRotation(), FRotator(0.0, NewYaw, 0.0), InDelta, 100.f);
			Turret->SetRelativeRotation(NewRotation);
		}
	}
}

void AJamShipBase::BroadsidesTracking(AJamShipBase* InTarget)
{
	if (!IsValid(InTarget)) return;
	if (!PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Broadsides_Stbd"))) return;
	if (!PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Broadsides_Port"))) return;

	//FVector StbdLocation = PhysicsRoot->GetSocketLocation(TEXT("Broadsides_Stbd"));
	//FVector PortLocation = PhysicsRoot->GetSocketLocation(TEXT("Broadsides_Port"));

	bool bStbdAngleValid = false;
	bool bPortAngleValid = false;

	FVector DirectionToTarget = FVector(InTarget->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();
	//DrawDebugLine(GetWorld(), this->GetActorLocation(), FVector((DirectionToTarget * 2000.f) + this->GetActorLocation()), FColor::Purple, false, 0.1f, 0.f, 10.f);

	bStbdAngleValid = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorRightVector(), DirectionToTarget))) < 30.f;
	bPortAngleValid = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorRightVector() * -1.f, DirectionToTarget))) < 30.f;

	if (bStbdAngleValid) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("Stbd Broadsides angle valid!"));
	if (bPortAngleValid) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("Port Broadsides angle valid!"));

	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, FString::SanitizeFloat(StbdAngleToTarget));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::SanitizeFloat(PortAngleToTarget));

	//DrawDebugLine(GetWorld(), this->GetActorLocation(), FVector((this->GetActorRightVector() * 2000.f) + this->GetActorLocation()), FColor::Green, false, 0.1f, 0.f, 10.f);
	//DrawDebugLine(GetWorld(), this->GetActorLocation(), FVector((this->GetActorRightVector() * -2000.f) + this->GetActorLocation()), FColor::Red, false, 0.1f, 0.f, 10.f);
	
	//if (bIsTurretsAimedAtTarget && bIsTurretsInRange) DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Green, false, 0.1f, 0.f, 10.f);
	//else DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Red, false, 0.1f, 0.f, 10.f);
}

void AJamShipBase::ShipApplyDamage(float InDamage)
{
	float ArmorDamage = InDamage;
	if (bShieldEnabled && !bShieldDown)
	{
		float CurrentShieldOld = CurrentShield;
		CurrentShield -= InDamage;
		if (CurrentShield <= 0)
		{
			ArmorDamage = FMath::Abs(CurrentShieldOld - InDamage);
			bShieldDown = true;
		}
		else ArmorDamage = 0;
	}

	if (ArmorDamage > 0) ShieldRegenTimer = 0.f;

	CurrentArmor -= ArmorDamage;
	if (CurrentArmor <= 0) bIsDestroyed = true;
}