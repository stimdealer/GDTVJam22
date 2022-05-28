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

	NS_DevestatorBeam = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/VFX/NS_DevestatorBeam.NS_DevestatorBeam'"), nullptr, LOAD_None, nullptr);
	if (!ensure(NS_DevestatorBeam != nullptr)) return;
	NS_GatlingFire = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/VFX/NS_FighterGatling.NS_FighterGatling'"), nullptr, LOAD_None, nullptr);
	if (!ensure(NS_GatlingFire != nullptr)) return;
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
	ForwardTracking();

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
		SFXTurretsFiring(true);
	}
	else SFXTurretsFiring(false);

	if (IsValid(BroadsideTargetShip) && bBroadsidesInRange)
	{
		if (bStbdAngleValid || bPortAngleValid)
		{
			BroadsideTargetShip->ShipApplyDamage(BroadsidesFirepower);
			SFXBroadsidesFiring(true);
		}
		else SFXBroadsidesFiring(false);
	}
	else SFXBroadsidesFiring(false);

	if (IsValid(ForwardTargetShip) && bForwardInRange && bForwardAngleValid)
	{
		ForwardTargetShip->ShipApplyDamage(ForwardFirepower);
		SFXForwardFiring(true);
	}
	else SFXForwardFiring(false);
}

void AJamShipBase::MoveToDestination(float InDelta)
{
	if (!IsValid(PhysicsRoot)) return;
	if (bPhoenixInProgress) return;

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
			SFXThruster(true);
			NewMaxSpeed = MaxSpeed * 2.f;
			if (BoostThrusterOne) BoostThrusterOne->Activate();
			if (BoostThrusterTwo) BoostThrusterTwo->Activate();
			if (BoostThrusterThree) BoostThrusterThree->Activate();
			if (BoostThrusterFour) BoostThrusterFour->Activate();
		}
		else
		{
			SFXThruster(false);
			NewMaxSpeed = MaxSpeed;
			if (BoostThrusterOne) BoostThrusterOne->Deactivate();
			if (BoostThrusterTwo) BoostThrusterTwo->Deactivate();
			if (BoostThrusterThree) BoostThrusterThree->Deactivate();
			if (BoostThrusterFour) BoostThrusterFour->Deactivate();
		}			

		float Thrust = MaxSpeed * 100.f;

		if (bNPCControlled && Distance < NPCCloseDistance)
		{
			Force = Prim->GetComponentVelocity() * -4;
		}
		else if (Distance > 1000.0 && Momentum < NewMaxSpeed)
		{
			if (bIsBoosting) Thrust = Thrust * 2.f;

			Force = Prim->GetForwardVector() * Thrust * InDelta;
		}
		else
		{
			Force = Prim->GetComponentVelocity() * -2;
		}

		Force = FVector(
			FMath::Clamp(Force.X, -NewMaxSpeed, NewMaxSpeed),
			FMath::Clamp(Force.Y, -NewMaxSpeed, NewMaxSpeed),
			FMath::Clamp(Force.Z, -NewMaxSpeed, NewMaxSpeed)
		);

		CameraLead = (Prim->GetComponentVelocity() * 0.75f) + this->GetActorLocation();

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
			}

			FRotator NewRotation = FMath::RInterpConstantTo(Turret->GetRelativeRotation(), FRotator(0.0, NewYaw, 0.0), InDelta, 100.f);
			Turret->SetRelativeRotation(NewRotation);
		}
		else GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, TEXT("Fire socket not found"));
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

void AJamShipBase::ForwardTracking()
{
	if (!IsValid(ForwardTargetShip)) return;

	FVector DirectionToTarget = FVector(ForwardTargetShip->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();

	bForwardAngleValid = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorForwardVector(), DirectionToTarget))) < 30.f;

	bForwardInRange = FVector::Distance(ForwardTargetShip->GetActorLocation(), this->GetActorLocation()) < ForwardRange;
}

void AJamShipBase::ShipApplyDamage(float InDamage)
{
	if (bPhoenixInProgress)
	{
		SFXImpactDamage(true);
		return;
	}

	bShieldCooldown = true;

	GetWorldTimerManager().ClearTimer(ShieldCooldownHandle);
	GetWorldTimerManager().SetTimer(ShieldCooldownHandle, ShieldCooldownDelegate, 10.f, false);

	float ArmorDamage = InDamage;
	if (bShieldEnabled)
	{
		float CurrentShieldOld = CurrentShield;
		CurrentShield -= InDamage;
		if (CurrentShield <= 0)
		{
			CurrentShield = 0;
			ArmorDamage = FMath::Abs(CurrentShieldOld - InDamage);
			SFXImpactDamage(false);
		}
		else
		{
			ArmorDamage = 0;
			SFXImpactDamage(true);
		}
	}
	else SFXImpactDamage(false);

	CurrentArmor -= ArmorDamage;
	if (CurrentArmor <= 0) bIsDestroyed = true;
}

void AJamShipBase::OnShieldCooldownComplete()
{
	bShieldCooldown = false;
}

void AJamShipBase::UpdateVFX()
{
	if (bIsTurretsAimedAtTarget && bIsTurretsInRange && IsValid(TurretTargetShip))
	{
		if (TurretOneVFX && TurretTwoVFX)
		{
			TurretOneVFX->SetNiagaraVariableVec3(TEXT("TargetLocation"), TurretTargetShip->GetActorLocation());
			TurretOneVFX->Activate();
			TurretTwoVFX->SetNiagaraVariableVec3(TEXT("TargetLocation"), TurretTargetShip->GetActorLocation());
			TurretTwoVFX->Activate();
		}
		else if (TurretHunterVFX)
		{
			TurretHunterVFX->SetNiagaraVariableVec3(TEXT("TargetLocation"), TurretTargetShip->GetActorLocation());
			TurretHunterVFX->Activate();
		}
	}
	else
	{
		if (TurretOneVFX && TurretTwoVFX)
		{
			TurretOneVFX->Deactivate();
			TurretTwoVFX->Deactivate();
		}
		else if (TurretHunterVFX) TurretHunterVFX->Deactivate();
	}

	if (NS_BroadsidesFire && BroadsidesStbdVFX && BroadsidesPortVFX)
	{
		if (IsValid(BroadsideTargetShip) && bBroadsidesInRange && bStbdAngleValid) BroadsidesStbdVFX->Activate();
		else BroadsidesStbdVFX->Deactivate();

		if (IsValid(BroadsideTargetShip) && bBroadsidesInRange && bPortAngleValid) BroadsidesPortVFX->Activate();
		else BroadsidesPortVFX->Deactivate();
	}

	if (GatlingFighterVFX)
	{
		if (IsValid(ForwardTargetShip) && bForwardInRange && bForwardAngleValid) GatlingFighterVFX->Activate();
		else GatlingFighterVFX->Deactivate();
	}

	if (DevestatorOneVFX && DevestatorTwoVFX)
	{
		if (IsValid(ForwardTargetShip) && bForwardInRange && bForwardAngleValid)
		{
			DevestatorOneVFX->Activate();
			DevestatorOneVFX->SetNiagaraVariableVec3(TEXT("TargetLocation"), ForwardTargetShip->GetActorLocation());
			DevestatorTwoVFX->Activate();
			DevestatorTwoVFX->SetNiagaraVariableVec3(TEXT("TargetLocation"), ForwardTargetShip->GetActorLocation());
		}
		else
		{
			DevestatorOneVFX->Deactivate();
			DevestatorTwoVFX->Deactivate();
		}
	}
}

void AJamShipBase::SpawnWeaponsVFX()
{
	if (NS_DevestatorBeam && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Devestator_1")))
	{
		DevestatorOneVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_DevestatorBeam,
			PhysicsRoot,
			FName("Devestator_1"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
	}

	if (NS_DevestatorBeam && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Devestator_2")))
	{
		DevestatorTwoVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_DevestatorBeam,
			PhysicsRoot,
			FName("Devestator_2"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
	}

	if (NS_GatlingFire && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Gatling_Fighter")))
	{
		GatlingFighterVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_GatlingFire,
			PhysicsRoot,
			FName("Gatling_Fighter"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
	}

	if (NS_TurretBeam && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Turret_Hunter")))
	{
		TurretHunterVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS_TurretBeam,
			PhysicsRoot,
			FName("Turret_Hunter"),
			FVector(0.0),
			FRotator(0.0),
			EAttachLocation::KeepRelativeOffset,
			false,
			false
		);
		TurretHunterVFX->SetNiagaraVariableLinearColor(TEXT("BeamColor"), FLinearColor(100.f, 0.f, 0.f, 1.f));
	}

	if (NS_TurretBeam && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Turret_1")))
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

	if (NS_TurretBeam && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Turret_2")))
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

	if (NS_BroadsidesFire && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Broadsides_Port")))
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

	if (NS_BroadsidesFire && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Broadsides_Stbd")))
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

	if (NS_ThrusterTrail && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_1")))
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

	if (NS_ThrusterTrail && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_2")))
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

	if (NS_ThrusterTrail && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_3")))
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

	if (NS_ThrusterTrail && PhysicsRoot->GetStaticMesh()->FindSocket(TEXT("Boost_4")))
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

float AJamShipBase::CalculatePercent(float InCurrent, float InMax)
{
	if (FMath::IsNearlyZero(InCurrent, 0.001)) return 0.f;
	else return InCurrent / InMax;
}