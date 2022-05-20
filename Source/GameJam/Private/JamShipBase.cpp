// Fill out your copyright notice in the Description page of Project Settings.


#include "JamShipBase.h"

// Sets default values
AJamShipBase::AJamShipBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PhysicsRoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Physics Root"));
	PhysicsRoot->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));
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

	MoveToDestination(DeltaTime);
}

void AJamShipBase::FireWeapons()
{
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, TEXT("Firing weapons!"));
}

void AJamShipBase::MoveToDestination(float InDelta)
{
	if (!IsValid(PhysicsRoot)) return;

	FVector Heading = FVector(Destination - this->GetActorLocation()).GetSafeNormal();

	float Distance = FVector::Distance(this->GetActorLocation(), Destination);
	float AngleDistance = FMath::RadiansToDegrees(acosf(FVector::DotProduct(this->GetActorForwardVector(), Heading)));

	UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(PhysicsRoot);
	if (IsValid(Prim))
	{
		// Apply rotation
		if (Distance > 1000.f) Prim->SetWorldRotation(FMath::RInterpTo(Prim->GetComponentRotation(), Heading.Rotation(), InDelta, 1.f));

		float Momentum;
		FVector Direction;
		Prim->GetComponentVelocity().ToDirectionAndLength(Direction, Momentum);

		// Apply forward thrust
		FVector Force = FVector(0.0);
		if (Distance > 400.0 && Momentum < MaxSpeed * 1.25 && AngleDistance < 10.0)
		{
			if (Distance > 1000.0) Thrust = 50000.f;
			else if (Distance > 600.0) Thrust = 20000.f;
			else Thrust = 10000.f;

			Force = Prim->GetForwardVector() * Thrust * InDelta;
		}
		else
		{
			if (Prim->GetComponentVelocity().X > 5.0 || Prim->GetComponentVelocity().Y > 5.0 || Prim->GetComponentVelocity().Z > 5.0)
			{
				Force = Prim->GetComponentVelocity() * -5;
			}
		}

		Force = FVector(
			FMath::Clamp(Force.X, -MaxSpeed, MaxSpeed),
			FMath::Clamp(Force.Y, -MaxSpeed, MaxSpeed),
			FMath::Clamp(Force.Z, -MaxSpeed, MaxSpeed)
		);

		Prim->AddForce(Force, NAME_None, true);
	}
}
