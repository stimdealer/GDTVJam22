// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JamShipBase.h"
#include "FighterShip.generated.h"

UCLASS()
class GAMEJAM_API AFighterShip : public AJamShipBase
{
	GENERATED_BODY()
	
public:
	AFighterShip();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void AssignTarget(class ANPCShip* InTarget, class APlayerShip* InPlayerShipRef);

private:
	class APlayerShip* PlayerShipRef = nullptr;

	bool bEngagingTarget = false;
	bool bReadyToReturn = false;

	float RefreshTimer = 0.f;

	void ReturnToPlayerShip();
};
