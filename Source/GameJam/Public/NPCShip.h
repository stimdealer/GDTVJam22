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

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Target Arrows")
	void ToggleArrows(bool IsVisible);

private:
	void CheckDistanceToPlayer();

	void EngagePlayer();
};