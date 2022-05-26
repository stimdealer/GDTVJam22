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
	AJamShipBase* FightersTargetShip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Destination;

	float TurretsFirepower = 0.f;
	float BroadsidesFirepower = 0.f;

	float MaxShield = 0.f;
	float CurrentShield = 0.f;

	float MaxArmor = 250.f;
	float CurrentArmor = 250.f;

	float MaxFuel = 100.f;
	float CurrentFuel = 100.f;

	float MaxSpeed = 1000.f;
	float TurnSpeed = 100.f;

	float TurretRange = 5000.f;
	float BroadsideRange = 7500.f;

	bool bShieldEnabled = false;
	bool bShieldCooldown = false;
	bool bIsBoosting = false;
	bool bIsDestroyed = false;

	bool bBroadsides = false;
	bool bLauncher = false;
	bool bFighters = false;
	bool bMissileReady = false;
	bool bFightersReady = false;

	bool bDestructionComplete = false;

	FVector CameraLead;

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXThruster(bool IsBoosting);

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXTurretsFiring(bool IsActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXBroadsidesFiring(bool IsActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "SFX Calls")
	void SFXImpactDamage(bool IsShieldImpact);

	UFUNCTION(BlueprintCallable)
	void SpawnWeaponsVFX();

	void FireWeapons();

private:
	UNiagaraSystem* NS_TurretBeam = nullptr;
	UNiagaraSystem* NS_BroadsidesFire = nullptr;
	UNiagaraSystem* NS_ThrusterTrail = nullptr;
	
	UNiagaraComponent* TurretHunterVFX = nullptr;
	UPROPERTY()
	UNiagaraComponent* TurretOneVFX = nullptr;
	UPROPERTY()
	UNiagaraComponent* TurretTwoVFX = nullptr;
	UPROPERTY()
	UNiagaraComponent* BroadsidesPortVFX = nullptr;
	UPROPERTY()
	UNiagaraComponent* BroadsidesStbdVFX = nullptr;
	UPROPERTY()
	UNiagaraComponent* BoostThrusterOne = nullptr;
	UPROPERTY()
	UNiagaraComponent* BoostThrusterTwo = nullptr;
	UPROPERTY()
	UNiagaraComponent* BoostThrusterThree = nullptr;
	UPROPERTY()
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

	float WeaponsTimer = 0.f;
	float MissileTimer = 0.f;

	float ShieldRegenTimer = 0.f;
	float ShieldRegenDelay = 0.f;

	void MoveToDestination(float InDelta);

	void TurretsTracking(float InDelta);
	void BroadsidesTracking();

	void UpdateVFX();
};
