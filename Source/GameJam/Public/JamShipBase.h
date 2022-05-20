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
	FVector Destination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PhysicsRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMesh* BaseMesh;

	UStaticMeshComponent* ShipBaseMesh = nullptr;

	UStaticMeshComponent* ShipUpgradeMesh = nullptr; // Placeholder.

	AJamShipBase* TargetShip = nullptr;

	virtual void BeginPlay() override;

	void FireWeapons();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	float MaxSpeed = 300.f;
	float Thrust = 50000.f;
	float TurnSpeed = 50.f;
	float WeaponsRange = 3000.f;

	void MoveToDestination(float InDelta);
};
