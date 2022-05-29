// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JamShipBase.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class GAMEJAM_API AJamShipBase : public APawn
{
	GENERATED_BODY()

public:
	AJamShipBase();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ShipApplyDamage(float InDamage);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PhysicsRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UStaticMeshComponent*> Turrets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AJamShipBase* TurretTargetShip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AJamShipBase* BroadsideTargetShip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AJamShipBase* ForwardTargetShip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Destination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPhoenixInProgress = false;

	bool bNPCControlled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NPCCloseDistance = 2500.f;

	// Ship base stats
	// Weapon Damage per .25 sec
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurretsFirepower = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BroadsidesFirepower = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardFirepower = 0.f;

	// Weapon Ranges
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurretRange = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BroadsideRange = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardRange = 3500.f;

	// Shield
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxShield = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentShield = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldRegenAmount = 100.f;

	// Armor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxArmor = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentArmor = 500.f;

	// Fuel - player only
	float MaxFuel = 100.f;
	float CurrentFuel = 100.f;

	// Mobility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurnSpeed = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShieldEnabled = false;

	bool bShieldCooldown = false;
	bool bIsBoosting = false;
	bool bIsDestroyed = false;

	bool bBroadsides = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bLauncher = false;

	bool bFighters = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMissileReady = false;

	bool bDestructionComplete = false;

	FVector CameraLead;

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXThruster(bool IsBoosting);

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXTurretsFiring(bool IsActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXBroadsidesFiring(bool IsActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXForwardFiring(bool IsActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXImpactDamage(bool IsShieldImpact);

	UFUNCTION(BlueprintCallable)
	void SpawnWeaponsVFX();

	void FireWeapons();

	float CalculatePercent(float InCurrent, float InMax);

private:
	UNiagaraSystem* NS_DevestatorBeam = nullptr;
	UNiagaraSystem* NS_GatlingFire = nullptr;
	UNiagaraSystem* NS_TurretBeam = nullptr;
	UNiagaraSystem* NS_BroadsidesFire = nullptr;
	UNiagaraSystem* NS_ThrusterTrail = nullptr;

	UNiagaraComponent* DevestatorOneVFX = nullptr;
	UNiagaraComponent* DevestatorTwoVFX = nullptr;
	UNiagaraComponent* GatlingFighterVFX = nullptr;
	UNiagaraComponent* TurretHunterVFX = nullptr;
	UNiagaraComponent* TurretOneVFX = nullptr;
	UNiagaraComponent* TurretTwoVFX = nullptr;
	UNiagaraComponent* TurretBossOneVFX = nullptr;
	UNiagaraComponent* TurretBossTwoVFX = nullptr;
	UNiagaraComponent* TurretBossThreeVFX = nullptr;
	UNiagaraComponent* TurretBossFourVFX = nullptr;
	UNiagaraComponent* TurretBossFiveVFX = nullptr;
	UNiagaraComponent* TurretBossSixVFX = nullptr;
	UNiagaraComponent* BroadsidesPortVFX = nullptr;
	UNiagaraComponent* BroadsidesStbdVFX = nullptr;
	UNiagaraComponent* BoostThrusterOne = nullptr;
	UNiagaraComponent* BoostThrusterTwo = nullptr;
	UNiagaraComponent* BoostThrusterThree = nullptr;
	UNiagaraComponent* BoostThrusterFour = nullptr;

	FTimerHandle ShieldCooldownHandle;
	FTimerDelegate ShieldCooldownDelegate;
	UFUNCTION()
	void OnShieldCooldownComplete();

	bool bIsTurretsAimedAtTarget = false;
	bool bIsTurretsInRange = false;

	bool bStbdAngleValid = false;
	bool bPortAngleValid = false;
	bool bBroadsidesInRange = false;

	bool bForwardAngleValid = false;
	bool bForwardInRange = false;

	float WeaponsTimer = 0.f;
	float MissileTimer = 0.f;

	float ShieldRegenTimer = 0.f;
	float ShieldRegenDelay = 0.f;

	void MoveToDestination(float InDelta);

	void TurretsTracking(float InDelta);
	void BroadsidesTracking();
	void ForwardTracking();

	void UpdateVFX();
};
