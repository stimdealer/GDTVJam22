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

	UPROPERTY()
	FString MarkerTitle;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	USceneComponent* Arrow = nullptr;
};

UCLASS()
class GAMEJAM_API APlayerShip : public AJamShipBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 UpgradeLevel = 1;

	int32 FighterCount = 6;

	APlayerShip();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void ApplyLootableBonus(int32 InType, int32 InAmount);

	UFUNCTION(BlueprintCallable)
	void ManualSelectTarget(AJamShipBase* InNewTarget);

	UFUNCTION(BlueprintCallable)
	void AddQuest(const FString& InQuestName, FVector InLocation);

	UFUNCTION(BlueprintCallable)
	void CompleteQuest(const FString& InQuestName);

	UFUNCTION(BlueprintCallable)
	bool GetFightersStatus();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USphereComponent* TargetField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AJamShipBase*> DeployedFighters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxFighters = 6;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ship Functions")
	void LaunchFighter(ANPCShip* InTarget);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ship Functions")
	void PhoenixExplosion();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendArmorFuelToUI(float InShieldPercent, float InArmorPercent, float InFuelPercent, float InOrePercent, float InPhoenixPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendIconsToUI(bool InBroadsides, bool InMissiles, bool InFighters, bool InMissileReady);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendMessageToUI(const FText& InMessage, bool bCommMessage = false);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void ShipPermanentDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI Functions")
	void SendUpgradeLevelToUI(int32 InLevel);

private:
	TArray<AJamShipBase*> AllTargets;

	ANPCShip* ClosestNPCShipTarget = nullptr;

	USceneComponent* CameraAttach;
	USceneComponent* CameraAzimuth;
	USceneComponent* CameraElevation;
	UCameraComponent* TopDownCamera;

	UStaticMesh* QuestArrowMesh = nullptr;

	TArray<FQuestMarker> QuestMarkers;

	FTimerHandle PhoenixInvulnHandle;

	bool bManualTargetSelected = false;

	float ScanFrequency = 0.25f;
	float ScanTimer = 0.f;

	bool bPhoenixReady = true;
	float PhoenixTimer = 59.99f;

	int32 MaxOre = 100;
	int32 CurrentOre = 0;

	float IconRadius = 60.f;

	// Controls
	void InputCameraZoomIn();
	void InputCameraZoomOut();
	void InputStartSpeedBoost();
	void InputStopSpeedBoost();

	void ScanForTargets();
	void SelectClosestTarget();

	// Misc functions
	void UpgradeShip(int32 InOreOverflow = 0, bool IsTierOneReset = false);
	void UpdateFighters();

	void UpdateQuestArrows();
	USceneComponent* SpawnQuestArrow();
};
