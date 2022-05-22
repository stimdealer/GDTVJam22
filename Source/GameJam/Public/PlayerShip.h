// © 2022. stimdealer & boban. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "JamShipBase.h"
#include "PlayerShip.generated.h"

class USceneComponent;
class UCameraComponent;
class USphereComponent;

class ANPCShip;

UCLASS()
class GAMEJAM_API APlayerShip : public AJamShipBase
{
	GENERATED_BODY()

public:
	APlayerShip();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendArmorFuelToUI(float InShieldPercent, float InArmorPercent, float InFuelPercent, float InPhoenixPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendMessageToUI(const FText& InMessage, bool DisplayImmediately = false);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void ShipPermanentDeath();

	UFUNCTION(BlueprintCallable)
	void ApplyLootableBonus(float InArmor, float InFuel);

	UFUNCTION(BlueprintCallable)
	void ManualSelectTarget(ANPCShip* InNewTarget);

	UFUNCTION(BlueprintCallable)
	void UpgradeShip(bool IsTierOneReset = false);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USphereComponent* TargetField;

	UStaticMesh* ShipTier4 = nullptr;
	UStaticMesh* ShipTier3 = nullptr;
	UStaticMesh* ShipTier2 = nullptr;
	UStaticMesh* ShipTier1 = nullptr;

	UStaticMesh* TurretLaser = nullptr;

private:
	TArray<ANPCShip*> AllTargets;
	ANPCShip* ClosestTarget = nullptr;

	USceneComponent* CameraAttach;
	USceneComponent* CameraAzimuth;
	USceneComponent* CameraElevation;
	UCameraComponent* TopDownCamera;

	float ScanFrequency = 0.5f;
	float ScanTimer = 0.f;

	bool bPhoenixReady = true;
	float PhoenixTimer = 0.f;

	int32 UpgradeLevel = 4;

	// Controls
	void InputCameraZoomIn();
	void InputCameraZoomOut();
	void InputStartSpeedBoost();
	void InputStopSpeedBoost();
	void InputFireMissile();
	void InputLaunchFighters();

	void ScanForTargets();
	void SelectClosestTarget();
};
