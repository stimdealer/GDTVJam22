// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackingMissile.h"

#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "JamShipBase.h"

// Sets default values
ATrackingMissile::ATrackingMissile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MissileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Missile Mesh"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MissileMeshObject(TEXT("StaticMesh'/Game/Models/Spaceships/Weapons/SM_MissileA.SM_MissileA'"));
	if (MissileMeshObject.Object)
	{
		MissileMesh->SetStaticMesh(MissileMeshObject.Object);
		MissileMesh->SetWorldScale3D(FVector(0.5f));
		MissileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		this->RootComponent = MissileMesh;
	}
}

// Called every frame
void ATrackingMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveTowardsTarget(DeltaTime);

	if (IsValid(Target) && FVector::Distance(Target->GetActorLocation(), this->GetActorLocation()) < ExplodeRadius) Explode();
}

void ATrackingMissile::BeginPlay()
{
	Super::BeginPlay();

	FTimerDelegate MissileLifeDelegate;

	MissileLifeDelegate.BindUFunction(this, FName("MissileLifeExpired"));

	GetWorldTimerManager().SetTimer(MissileLifeHandle, MissileLifeDelegate, 10.f, false);

	NS_Explosion = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/VFX/NS_GeneralExplosion.NS_GeneralExplosion'"), nullptr, LOAD_None, nullptr);
	if (!ensure(NS_Explosion != nullptr)) return;
	auto NS_Thruster = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/VFX/NS_ThrusterTrail.NS_ThrusterTrail'"), nullptr, LOAD_None, nullptr);
	if (!ensure(NS_Thruster != nullptr)) return;

	if (IsValid(MissileMesh))
	{
		if (MissileMesh->GetStaticMesh()->FindSocket(TEXT("Thruster")))
		{
			auto ThrusterVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
				NS_Thruster,
				MissileMesh,
				FName("Thruster"),
				FVector(0.0),
				FRotator(0.0),
				EAttachLocation::KeepRelativeOffset,
				true
			);
			ThrusterVFX->SetNiagaraVariableFloat(TEXT("Width"), 4.f);
			ThrusterVFX->SetNiagaraVariableLinearColor(TEXT("Color"), FLinearColor(100.f, 50.f, 0.f, 1.f));
		}
	}
}

void ATrackingMissile::MoveTowardsTarget(float InDelta)
{
	if (!IsValid(Target)) return;

	FVector Direction = FVector(Target->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();
	this->SetActorRotation(Direction.Rotation());

	FVector Translate = FVector(Direction * MissileSpeed * InDelta);
	this->AddActorWorldOffset(Translate);
}

void ATrackingMissile::Explode()
{
	Target->ShipApplyDamage(150.f);

	auto ExplosionVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		NS_Explosion,
		(this->GetActorForwardVector() * 500.f) + this->GetActorLocation(),
		FRotator(0.0),
		FVector(1.f),
		true,
		true
	);

	this->Destroy();
}

void ATrackingMissile::MissileLifeExpired()
{
	this->Destroy();
}