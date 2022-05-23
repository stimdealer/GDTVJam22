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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurretsFirepower = 10.f;

	float MaxShield = 200.f;
	float CurrentShield = 200.f;

	float MaxArmor = 200.f;
	float CurrentArmor = 200.f;

	float MaxFuel = 100.f;
	float CurrentFuel = 100.f;

	float MaxSpeed = 1000.f;
	float TurnSpeed = 120.f;

	bool bShieldEnabled = true;
	bool bShieldDown = false;
	bool bIsBoosting = false;
	bool bIsDestroyed = false;

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

	float Thrust = 50000.f;
	float WeaponsRange = 6000.f;

	void MoveToDestination(float InDelta);

	void TurretsTracking(float InDelta);
};
