// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCShip.h"

ANPCShip::ANPCShip()
{
	PrimaryActorTick.bCanEverTick = true;

	TurretsFirepower = 5.f;
}

void ANPCShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDestroyed && !bDestructionComplete)
	{
		bDestructionComplete = true;
		ShipDeath();
	}
		
	else CheckDistanceToPlayer();

	UpdateStats(CurrentArmor / MaxArmor);
}

void ANPCShip::CheckDistanceToPlayer()
{
	if (!IsValid(PlayerShipRef)) return;

	float Distance = FVector::Distance(this->GetActorLocation(), PlayerShipRef->GetActorLocation());

	if (Distance > 20000)
	{
		TurretTargetShip = nullptr;
		ToggleTurretArrows(false);
		ToggleBroadsideArrows(false);
	}
	else
	{
		EngagePlayer();
	}		
}

void ANPCShip::EngagePlayer()
{
	TurretTargetShip = PlayerShipRef;
	if (IsValid(TurretTargetShip)) Destination = TurretTargetShip->GetActorLocation();
}
