// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCShip.h"

void ANPCShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDestroyed) this->Destroy();
	else CheckDistanceToPlayer();
}

void ANPCShip::CheckDistanceToPlayer()
{
	if (!IsValid(PlayerShipRef)) return;

	float Distance = FVector::Distance(this->GetActorLocation(), PlayerShipRef->GetActorLocation());

	if (Distance < AggroRadius)
	{
		EngagePlayer();
	}
}

void ANPCShip::EngagePlayer()
{
	TargetShip = PlayerShipRef;
	Destination = PlayerShipRef->GetActorLocation();
	FireWeapons();
}
