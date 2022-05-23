// © 2022. stimdealer & boban. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "JamShipBase.h"
#include "PlayerShip.generated.h"

class USceneComponent;
class UCameraComponent;
class USphereComponent;

class ANPCShip;

USTRUCT(BlueprintType)
struct GAMEJAM_API FQuestMarker
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString MarkerTitle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bReached;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCompleted;
};

UCLASS()
class GAMEJAM_API APlayerShip : public AJamShipBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 UpgradeLevel = 4;

	APlayerShip();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void ApplyLootableBonus(int32 InType, int32 InAmount);

	UFUNCTION(BlueprintCallable)
	void ManualSelectTarget(ANPCShip* InNewTarget);

	UFUNCTION(BlueprintCallable)
	void UpgradeShip(bool IsTierOneReset = false);

	UFUNCTION(BlueprintCallable)
	void AddQuest(const FString& InQuestName, FVector InLocation);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USphereComponent* TargetField;

	UStaticMesh* ShipTier4 = nullptr;
	UStaticMesh* ShipTier3 = nullptr;
	UStaticMesh* ShipTier2 = nullptr;
	UStaticMesh* ShipTier1 = nullptr;

	UStaticMesh* TurretLaser = nullptr;


	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendArmorFuelToUI(float InShieldPercent, float InArmorPercent, float InFuelPercent, float InPhoenixPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendMessageToUI(const FText& InMessage, bool DisplayImmediately = false);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void ShipPermanentDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendUpgradeLevelToUI(int32 InLevel);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Markers")
	void UpdateQuestMarkerUI(const FQuestMarker& InMarker, FVector2D InPosition, bool IsEnlargedIcon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Markers")
	void RemoveQuestMarkerUI(const FQuestMarker& InMarker);

private:
	TArray<ANPCShip*> AllTargets;
	ANPCShip* ClosestTarget = nullptr;

	USceneComponent* CameraAttach;
	USceneComponent* CameraAzimuth;
	USceneComponent* CameraElevation;
	UCameraComponent* TopDownCamera;

	TArray<FQuestMarker> ActiveQuestMarkers;

	float ScanFrequency = 0.5f;
	float ScanTimer = 0.f;

	bool bPhoenixReady = true;
	float PhoenixTimer = 0.f;

	bool bBroadsides = true;
	bool bLauncher = true;
	bool bFighters = true;

	int32 MaxOre = 0;
	int32 CurrentOre = 0;

	// UI Icons
	void UpdateQuestMarkers();

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
