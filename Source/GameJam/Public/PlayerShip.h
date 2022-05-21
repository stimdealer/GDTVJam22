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
	void SendArmorFuelToUI(float InArmorPercent, float InFuelPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendMessageToUI(const FText& InMessage);

	void UpgradeShip();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USphereComponent* TargetField;

private:
	TArray<ANPCShip*> AllTargets;
	ANPCShip* ClosestTarget = nullptr;

	USceneComponent* CameraAttach;
	USceneComponent* CameraAzimuth;
	USceneComponent* CameraElevation;
	UCameraComponent* TopDownCamera;

	bool bPrimaryPressed;
	float ScanFrequency = 0.5f;
	float ScanTimer = 0.f;

	// Controls
	void StartLeftClick();
	void StopLeftClick();
	void InputCameraZoomIn();
	void InputCameraZoomOut();
	void InputStartSpeedBoost();
	void InputStopSpeedBoost();

	void ScanForTargets();
	void SelectClosestTarget();
};
