// © 2022. stimdealer & boban. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "JamShipBase.h"
#include "PlayerShip.generated.h"

class USceneComponent;
class UCameraComponent;

UCLASS()
class GAMEJAM_API APlayerShip : public AJamShipBase
{
	GENERATED_BODY()

public:
	APlayerShip();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Destination;

private:
	USceneComponent* CameraAttach;
	USceneComponent* CameraAzimuth;
	USceneComponent* CameraElevation;
	UCameraComponent* TopDownCamera;

	bool bPrimaryPressed;
	float MaxSpeed = 300.f;
	float Thrust = 50000.f;
	float TurnSpeed = 50.f;

	void MoveToDestination(float InDelta);

	// Mouse controls
	void StartLeftClick();
	void StopLeftClick();
	void StartRightClick();
	void StopRightClick();

	void InputNavMouseX(float Value);
	void InputNavMouseY(float Value);
	void InputCameraZoom(float Value);
};
