// Fill out your copyright notice in the Description page of Project Settings.


#include "JamShipBase.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AJamShipBase::AJamShipBase()
{
	PrimaryActorTick.bCanEverTick = true;

	NS_TurretBeam = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/VFX/NS_TurretFire.NS_TurretFire'"), nullptr, LOAD_None, nullptr);
	if (!ensure(NS_TurretBeam != nullptr)) return;
	NS_BroadsidesFire = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/VFX/NS_BroadsidesFire.NS_BroadsidesFire'"), nullptr, LOAD_None, nullptr);
	if (!ensure(NS_BroadsidesFire != nullptr)) return;
	NS_ThrusterTrail = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/VFX/NS_ThrusterTrail.NS_ThrusterTrail'"), nullptr, LOAD_None, nullptr);
	if (!ensure(NS_ThrusterTrail != nullptr)) return;

	PhysicsRoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Physics Root"));
	PhysicsRoot->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));
	this->RootComponent = PhysicsRoot;
	PhysicsRoot->SetSimulatePhysics(false);
}

void AJamShipBase::BeginPlay()
{
	Super::BeginPlay();
	
	ShieldCooldownDelegate.BindUFunction(this, FName("OnShieldCooldownComplete"));

	SpawnWeaponsVFX();
}

// Called every frame
void AJamShipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateVFX();

	if (bIsDestroyed) return;

	MoveToDestination(DeltaTime);
	TurretsTracking(DeltaTime);
	BroadsidesTracking();

	if (bIsBoosting) CurrentFuel -= DeltaTime;

	if (bShieldCooldown) ShieldRegenDelay += DeltaTime;
	else ShieldRegenTimer += DeltaTime;

	if (!bShieldCooldown && ShieldRegenTimer > 2.5f)
	{
		ShieldRegenTimer = 0.f;
		CurrentShield += MaxShield * 0.1f;
		CurrentShield = FMath::Clamp(CurrentShield, 0, MaxShield);
	}

	WeaponsTimer += DeltaTime;
	if (WeaponsTimer > 0.25f)
	{
		WeaponsTimer = 0.f;
		FireWeapons();
	}
}

void AJamShipBase::FireWeapons()
{
	if (IsValid(TurretTargetShip) && bIsTurretsInRange && bIsTurretsAimedAtTarget)
	{
		TurretTargetShip->ShipApplyDamage(TurretsFirepower);
	}

	if (IsValid(BroadsideTargetShip) && bBroadsidesInRange)
	{
		if (bStbdAngleValid || bPortAngleValid)
		{
			BroadsideTargetShip->ShipApplyDamage(BroadsidesFirepower);
		}
	}		
}

void AJamShipBase::MoveToDestination(float InDelta)
{
	if (!IsValid(PhysicsRoot)) return;

	FVector Heading = FVector(Destination - this->GetActorLocation()).GetSafeNormal();

	float Distance = FVector::Distance(this->GetActorLocation(), Destination);
	float AngleDistance = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorForwardVector(), Heading)));

	UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(PhysicsRoot);

	if (IsValid(Prim) && Prim->IsSimulatingPhysics())
	{
		// Apply rotation
		Prim->SetWorldRotation(FMath::RInterpTo(Prim->GetComponentRotation(), Heading.Rotation(), InDelta, float(TurnSpeed / Prim->GetMass())));

		float Momentum;
		FVector Direction;
		Prim->GetComponentVelocity().ToDirectionAndLength(Direction, Momentum);

		// Apply forward thrust
		FVector Force = FVector(0.0);
		float NewMaxSpeed;
		if (bIsBoosting && CurrentFuel > 0.f)
		{
			NewMaxSpeed = MaxSpeed * 2.f;
			if (BoostThrusterOne) BoostThrusterOne->Activate();
			if (BoostThrusterTwo) BoostThrusterTwo->Activate();
			if (BoostThrusterThree) BoostThrusterThree->Activate();
			if (BoostThrusterFour) BoostThrusterFour->Activate();
		}
		else
		{
			NewMaxSpeed = MaxSpeed;
			if (BoostThrusterOne) BoostThrusterOne->Deactivate();
			if (BoostThrusterTwo) BoostThrusterTwo->Deactivate();
			if (BoostThrusterThree) BoostThrusterThree->Deactivate();
			if (BoostThrusterFour) BoostThrusterFour->Deactivate();
		}			

		float Thrust = MaxSpeed * 100.f;

		if (Distance > 1000.0 && Momentum < NewMaxSpeed)
		{
			if (bIsBoosting) Thrust = Thrust * 2.f;
			//else if (Distance < 1000.0) Thrust = 50000.f;

			Force = Prim->GetForwardVector() * Thrust * InDelta;
		}
		else
		{
			Force = Prim->GetComponentVelocity() * -2;
			/*
			if (Prim->GetComponentVelocity().X > 5.0 || Prim->GetComponentVelocity().Y > 5.0 || Prim->GetComponentVelocity().Z > 5.0)
			{
				Force = Prim->GetComponentVelocity() * -2;
			}
			*/
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
			if (IsValid(TurretTargetShip))
			{
				FVector TargetLocation = TurretTargetShip->GetActorLocation();
				FTransform TurretTransform = this->GetActorTransform();
				FVector Translate = TargetLocation - Turret->GetComponentLocation();
				FVector Inverse = UKismetMathLibrary::InverseTransformDirection(TurretTransform, Translate);
				FRotator RotFromX = UKismetMathLibrary::MakeRotFromX(Inverse);
				NewYaw = RotFromX.Yaw;

				float TurretAngleToTarget = FMath::RadiansToDegrees(acosf(FVector::DotProduct(Turret->GetForwardVector(), Translate.GetSafeNormal())));
				bIsTurretsAimedAtTarget = TurretAngleToTarget < 30.f;

				bIsTurretsInRange = FVector::Distance(TurretTargetShip->GetActorLocation(), this->GetActorLocation()) < TurretRange;

				// Debug lines for testing
				//if (bIsTurretsAimedAtTarget && bIsTurretsInRange) DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Green, false, 0.1f, 0.f, 10.f);
				//else DrawDebugLine(GetWorld(), Turret->GetSocketLocation(TEXT("Fire")), TargetShip->GetActorLocation(), FColor::Red, false, 0.1f, 0.f, 10.f);
			}

			FRotator NewRotation = FMath::RInterpConstantTo(Turret->GetRelativeRotation(), FRotator(0.0, NewYaw, 0.0), InDelta, 100.f);
			Turret->SetRelativeRotation(NewRotation);
		}
	}
}

void AJamShipBase::BroadsidesTracking()
{
	if (!IsValid(BroadsideTargetShip)) return;

	FVector DirectionToTarget = FVector(BroadsideTargetShip->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();

	bStbdAngleValid = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorRightVector(), DirectionToTarget))) < 30.f;
	bPortAngleValid = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorRightVector() * -1.f, DirectionToTarget))) < 30.f;

	bBroadsidesInRange = FVector::Distance(BroadsideTargetShip->GetActorLocation(), this->GetActorLocation()) < BroadsideRange;
}

void AJamShipBase::ShipApplyDamage(float InDamage)
{
	bShieldCooldown = true;

	GEngine->AddOnScreenDebugMessage(-1, 0.25f, FColor::White, FString::SanitizeFloat(InDamage));

	GetWorldTimerManager().ClearTimer(ShieldCooldownHandle);
	GetWorldTimerManager().SetTimer(ShieldCooldownHandle, ShieldCooldownDelegate, 10.f, false);

	float ArmorDamage = InDamage;
	if (bShieldEnabled)
	{
		float CurrentShieldOld = CurrentShield;
		CurrentShield -= InDamage;
		if (CurrentShield <= 0)
		{
			ArmorDamage = FMath::Abs(CurrentShieldOld - InDamage);
		}
		else ArmorDamage = 0;
	}

	CurrentArmor -= ArmorDamage;
	if (CurrentArmor <= 0) bIsDestroyed = true;
}

void AJamShipBase::OnShieldCooldownComplete()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("OnShieldCooldownComplete called!"));
	bShieldCooldown = false;
}

void AJamShipBase::UpdateVFX()
{
	if (NS_TurretBeam && TurretOneVFX && TurretTwoVFX)
	{
		if (bIsTurretsAimedAtTarget && bIsTurretsInRange && IsValid(TurretTargetShip))
		{
			TurretOneVFX->SetNiagaraVariableVec3(TEXT("TargetLocation"), TurretTargetShip->GetActorLocation());
			TurretOneVFX->Activate();
			TurretTwoVFX->SetNiagaraVariableVec3(TEXT("TargetLocation"), TurretTargetShip->GetActorLocation());
			TurretTwoVFX->Activate();
		}
		else
		{
			TurretOneVFX->Deactivate();
			TurretTwoVFX->Deactivate();
		}
	}
		
	if (NS_BroadsidesFire && BroadsidesStbdVFX && BroadsidesPortVFX)
	{
		if (IsValid(BroadsideTargetShip) && bBroadsidesInRange && bStbdAngleValid) BroadsidesStbdVFX->Activate();
		else BroadsidesStbdVFX->Deactivate();

		if (IsValid(BroadsideTargetShip) && bBroadsidesInRange && bPortAngleValid) BroadsidesPortVFX->Activate();
		else BroadsidesPortVFX->Deactivate();
	}
}

void AJamShipBase::SpawnWeaponsVFX()
{
	if (!NS_TurretBeam) return;
	if (!NS_BroadsidesFire) return;
	if (!NS_ThrusterTrail) return;

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Turret_1")))
	{
		TurretOneVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_TurretBeam,
			PhysicsRoot,
			FName("Turret_1"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
	}

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Turret_2")))
	{
		TurretTwoVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_TurretBeam,
			PhysicsRoot,
			FName("Turret_2"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
	}

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Broadsides_Port")))
	{
		BroadsidesPortVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_BroadsidesFire,
			PhysicsRoot,
			FName("Broadsides_Port"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::SnapToTarget,
			false,
			false
		);
	}

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Broadsides_Stbd")))
	{
		BroadsidesStbdVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_BroadsidesFire,
			PhysicsRoot,
			FName("Broadsides_Stbd"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::SnapToTarget,
			false,
			false
		);
	}

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_1")))
	{
		BoostThrusterOne = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_ThrusterTrail,
			PhysicsRoot,
			FName("Boost_1"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
		BoostThrusterOne->SetNiagaraVariableFloat(TEXT("Width"), 24.f);
		BoostThrusterOne->SetNiagaraVariableLinearColor(TEXT("Color"), FLinearColor(100.f, 50.f, 0.f, 1.f));
	}

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_2")))
	{
		BoostThrusterTwo = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_ThrusterTrail,
			PhysicsRoot,
			FName("Boost_2"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
		BoostThrusterTwo->SetNiagaraVariableFloat(TEXT("Width"), 24.f);
		BoostThrusterTwo->SetNiagaraVariableLinearColor(TEXT("Color"), FLinearColor(100.f, 50.f, 0.f, 1.f));
	}

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_3")))
	{
		BoostThrusterThree = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_ThrusterTrail,
			PhysicsRoot,
			FName("Boost_3"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
		BoostThrusterThree->SetNiagaraVariableFloat(TEXT("Width"), 24.f);
		BoostThrusterThree->SetNiagaraVariableLinearColor(TEXT("Color"), FLinearColor(100.f, 50.f, 0.f, 1.f));
	}

	if (PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_4")))
	{
		BoostThrusterFour = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_ThrusterTrail,
			PhysicsRoot,
			FName("Boost_4"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
		BoostThrusterFour->SetNiagaraVariableFloat(TEXT("Width"), 24.f);
		BoostThrusterFour->SetNiagaraVariableLinearColor(TEXT("Color"), FLinearColor(100.f, 50.f, 0.f, 1.f));
	}
}