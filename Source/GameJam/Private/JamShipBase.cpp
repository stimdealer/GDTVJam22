// Fill out your copyright notice in the Description page of Project Settings.


#include "JamShipBase.h"

// Sets default values
AJamShipBase::AJamShipBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PhysicsRoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Physics Root"));
	this->RootComponent = PhysicsRoot;
	PhysicsRoot->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void AJamShipBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJamShipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AJamShipBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

