// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JamShipBase.generated.h"

UCLASS()
class GAMEJAM_API AJamShipBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AJamShipBase();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PhysicsRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UStaticMeshComponent*> Turrets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AJamShipBase* TargetShip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Destination;

	// Default stats set up for small NPC ship.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurretsFirepower = 5.f;

	float MaxShield = 0.f;
	float CurrentShield = 0.f;

	float MaxArmor = 100.f;
	float CurrentArmor = 100.f;

	float MaxFuel = 100.f;
	float CurrentFuel = 100.f;

	float MaxSpeed = 500.f;
	float TurnSpeed = 80.f;

	bool bShieldEnabled = false;
	bool bShieldDown = false;
	bool bIsBoosting = false;
	bool bIsDestroyed = false;

	bool bBroadsides = false;
	bool bLauncher = false;
	bool bFighters = false;

	void FireTurrets();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ShipApplyDamage(float InDamage);

private:
	bool bIsTurretsAimedAtTarget = false;
	bool bIsTurretsInRange = false;

	float WeaponsTimer = 0.f;
	float ShieldRegenTimer = 0.f;
	float ShieldRegenDelay = 0.f;

	float WeaponsRange = 6000.f;

	void MoveToDestination(float InDelta);

	void TurretsTracking(float InDelta);
	void BroadsidesTracking(AJamShipBase* InTarget);
};
