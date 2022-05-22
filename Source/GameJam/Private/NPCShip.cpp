// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCShip.h"

void ANPCShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDestroyed) this->Destroy();
	else CheckDistanceToPlayer();

	UpdateStats(CurrentShield / MaxShield, CurrentArmor / MaxArmor);
}

void ANPCShip::CheckDistanceToPlayer()
{
	if (!IsValid(PlayerShipRef)) return;

	float Distance = FVector::Distance(this->GetActorLocation(), PlayerShipRef->GetActorLocation());

	if (Distance > 20000)
	{
		TargetShip = nullptr;
		ToggleArrows(false);
	}
	else
	{
		EngagePlayer();
	}		
}

void ANPCShip::EngagePlayer()
{
	TargetShip = PlayerShipRef;
	if (IsValid(TargetShip)) Destination = TargetShip->GetActorLocation();
}
