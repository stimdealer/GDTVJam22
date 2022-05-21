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
	FVector Destination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PhysicsRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMesh* BaseMesh;

	UStaticMeshComponent* ShipBaseMesh = nullptr;

	UStaticMeshComponent* ShipUpgradeMesh = nullptr; // Placeholder.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AJamShipBase* TargetShip = nullptr;

	bool bIsDestroyed = false;

	virtual void BeginPlay() override;

	void FireWeapons();

	void SetupTurret(FName InSocket, ETurretType InType);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ApplyDamage(float InDamage);

private:
	float WeaponsTimer = 0.f;
	float TurretsFirepower = 20.f;

	float MaxHealth = 100.f;
	float CurrentHealth = 100.f;

	float MaxSpeed = 300.f;
	float Thrust = 50000.f;
	float TurnSpeed = 50.f;
	float WeaponsRange = 3000.f;

	void MoveToDestination(float InDelta);

	void TurretsTracking(float InDelta);
};
