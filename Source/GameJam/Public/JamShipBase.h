// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JamShipBase.generated.h"

UENUM()
enum ETurretType { Laser, Cannon, Rocket };

UCLASS()
class GAMEJAM_API AJamShipBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AJamShipBase();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UStaticMeshComponent*> Turrets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PhysicsRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AJamShipBase* TargetShip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Destination;

	bool bIsBoosting = false;
	bool bIsDestroyed = false;

	void SetupTurret(FName InSocket, ETurretType InType);

	virtual void BeginPlay() override;

	void FireWeapons();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ShipApplyDamage(float InDamage);

	float GetHealthPercent();
	float GetFuelPercent();

private:
	bool bIsTurretsAimedAtTarget = false;
	bool bIsTurretsInRange = false;

	float WeaponsTimer = 0.f;
	float TurretsFirepower = 5.f;

	float MaxHealth = 100.f;
	float CurrentHealth = 100.f;

	float MaxFuel = 100.f;
	float CurrentFuel = 100.f;

	float MaxSpeed = 500.f;
	float Thrust = 50000.f;
	float TurnSpeed = 75.f;
	float WeaponsRange = 6000.f;

	void MoveToDestination(float InDelta);

	void TurretsTracking(float InDelta);
};
