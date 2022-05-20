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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMesh* BaseMesh;

	virtual void BeginPlay() override;

	UStaticMeshComponent* ShipBaseMesh = nullptr;

	UStaticMeshComponent* ShipUpgradeMesh = nullptr; // Placeholder.

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
