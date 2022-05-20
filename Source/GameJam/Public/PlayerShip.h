// © 2022. stimdealer & boban. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "JamShipBase.h"
#include "PlayerShip.generated.h"

class USceneComponent;
class UCameraComponent;
class USphereComponent;

UCLASS()
class GAMEJAM_API APlayerShip : public AJamShipBase
{
	GENERATED_BODY()

public:
	APlayerShip();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USphereComponent* TargetField;

private:
	USceneComponent* CameraAttach;
	USceneComponent* CameraAzimuth;
	USceneComponent* CameraElevation;
	UCameraComponent* TopDownCamera;

	bool bPrimaryPressed;
	float ScanFrequency = 0.5f;
	float ScanTimer = 0.f;

	// Mouse controls
	void StartLeftClick();
	void StopLeftClick();
	void StartRightClick();
	void StopRightClick();

	void InputNavMouseX(float Value);
	void InputNavMouseY(float Value);
	void InputCameraZoom(float Value);

	TArray<class ANPCShip*> ScanForTargets();
};
