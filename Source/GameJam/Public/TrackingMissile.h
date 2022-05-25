// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackingMissile.generated.h"

UCLASS()
class GAMEJAM_API ATrackingMissile : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrackingMissile();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AJamShipBase* Target = nullptr;

protected:
	virtual void BeginPlay() override;

private:
	UStaticMeshComponent* MissileMesh = nullptr;

	class UNiagaraSystem* NS_Explosion;

	FTimerHandle MissileLifeHandle;

	float MissileSpeed = 2000.f;
	float ExplodeRadius = 1000.f;

	void MoveTowardsTarget(float InDelta);
	void Explode();

	UFUNCTION()
	void MissileLifeExpired();
};