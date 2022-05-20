// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JamShipBase.h"
#include "NPCShip.generated.h"

UCLASS()
class GAMEJAM_API ANPCShip : public AJamShipBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AJamShipBase* PlayerShipRef = nullptr;

	float AggroRadius = 10000.f;

	virtual void Tick(float DeltaTime) override;

private:
	void CheckDistanceToPlayer();

	void EngagePlayer();
};
