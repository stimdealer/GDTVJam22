// Fill out your copyright notice in the Description page of Project Settings.


#include "FighterShip.h"

#include "PlayerShip.h"
#include "NPCShip.h"

AFighterShip::AFighterShip()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxSpeed = 2000.f;
	TurnSpeed = 350.f;

	ForwardFirepower = 10.f;
}

void AFighterShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bReadyToReturn && IsValid(PlayerShipRef)) Destination = PlayerShipRef->GetActorLocation();
	else if (IsValid(ForwardTargetShip)) Destination = ForwardTargetShip->GetActorLocation();

	RefreshTimer += DeltaTime;
	if (RefreshTimer > 0.5f)
	{
		RefreshTimer = 0.f;
		if (!IsValid(ForwardTargetShip) && bEngagingTarget) bReadyToReturn = true;
		
		if (IsValid(PlayerShipRef))
		{
			if (bReadyToReturn)
			{
				float DistanceToPlayer = FVector::Distance(this->GetActorLocation(), PlayerShipRef->GetActorLocation());
				if (DistanceToPlayer < 2500.f) this->Destroy();
			}
		}
		else this->Destroy();
	}
}

void AFighterShip::AssignTarget(ANPCShip* InTarget, APlayerShip* InPlayerShipRef)
{
	SpawnWeaponsVFX();
	if (IsValid(InPlayerShipRef)) PlayerShipRef = InPlayerShipRef;
	if (IsValid(InTarget)) ForwardTargetShip = Cast<AJamShipBase>(InTarget);
	bEngagingTarget = true;
}