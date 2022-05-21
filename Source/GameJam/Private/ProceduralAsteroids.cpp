// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralAsteroids.h"

#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AProceduralAsteroids::AProceduralAsteroids()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidAMeshObject(TEXT("StaticMesh'/Game/Models/Environment/Asteroids/SM_AsteroidA.SM_AsteroidA'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidBMeshObject(TEXT("StaticMesh'/Game/Models/Environment/Asteroids/SM_AsteroidB.SM_AsteroidB'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidCMeshObject(TEXT("StaticMesh'/Game/Models/Environment/Asteroids/SM_AsteroidC.SM_AsteroidC'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidDMeshObject(TEXT("StaticMesh'/Game/Models/Environment/Asteroids/SM_AsteroidD.SM_AsteroidD'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidEMeshObject(TEXT("StaticMesh'/Game/Models/Environment/Asteroids/SM_AsteroidE.SM_AsteroidE'"));

	AsteroidA = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Asteroid Type A"));
	AsteroidB = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Asteroid Type B"));
	AsteroidC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Asteroid Type C"));
	AsteroidD = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Asteroid Type D"));
	AsteroidE = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Asteroid Type E"));

	if (!ensure(AsteroidA != nullptr)) return;
	if (!ensure(AsteroidB != nullptr)) return;
	if (!ensure(AsteroidC != nullptr)) return;
	if (!ensure(AsteroidD != nullptr)) return;
	if (!ensure(AsteroidE != nullptr)) return;

	if (AsteroidAMeshObject.Object) AsteroidA->SetStaticMesh(AsteroidAMeshObject.Object);
	if (AsteroidBMeshObject.Object) AsteroidB->SetStaticMesh(AsteroidBMeshObject.Object);
	if (AsteroidCMeshObject.Object) AsteroidC->SetStaticMesh(AsteroidCMeshObject.Object);
	if (AsteroidDMeshObject.Object) AsteroidD->SetStaticMesh(AsteroidDMeshObject.Object);
	if (AsteroidEMeshObject.Object) AsteroidE->SetStaticMesh(AsteroidEMeshObject.Object);

	AsteroidA->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AsteroidA->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AsteroidB->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AsteroidB->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AsteroidC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AsteroidC->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AsteroidD->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AsteroidD->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AsteroidE->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AsteroidE->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AProceduralAsteroids::BeginPlay()
{
	Super::BeginPlay();

	AsteroidTypes.Add(AsteroidA);
	AsteroidTypes.Add(AsteroidB);
	AsteroidTypes.Add(AsteroidC);
	AsteroidTypes.Add(AsteroidD);
	AsteroidTypes.Add(AsteroidE);

	GenerateBeltPath(CurrentSeed);
}

void AProceduralAsteroids::GenerateBeltPath(int32 InSeed)
{
	AsteroidFieldStream.Initialize(InSeed);

	if (bBoxField) SpawnAsteroidsBox();
	else
	{
		for (int32 i = 0; i < BeltLength; ++i)
		{
			LastPoint += GenerateNextBeltPoint(BeltDirection);
			SpawnAsteroidInstances(LastPoint);
		}
	}
}

FVector AProceduralAsteroids::GenerateNextBeltPoint(FVector InDirection)
{
	float AngleVariation = 30.f;

	float NewPitch = AsteroidFieldStream.FRandRange(InDirection.Rotation().Pitch - AngleVariation, InDirection.Rotation().Pitch + AngleVariation);
	float NewYaw = AsteroidFieldStream.FRandRange(InDirection.Rotation().Yaw - AngleVariation, InDirection.Rotation().Yaw + AngleVariation);
	float NewRoll = AsteroidFieldStream.FRandRange(AngleVariation * -1.f, AngleVariation);

	FRotator NewRot = FRotator(NewPitch, NewYaw, NewRoll);
	FVector NewDirection = NewRot.RotateVector(InDirection);

	float NewDistance = AsteroidFieldStream.FRandRange(BeltPointDistance * 0.5f, BeltPointDistance * 1.5f);
	NewDirection *= NewDistance;

	return NewDirection;
}

void AProceduralAsteroids::SpawnAsteroidInstances(FVector InStartLocation)
{
	int32 RandomAmount = AsteroidFieldStream.FRandRange(MinAmount, MaxAmount);
	for (int32 i = 0; i < FMath::Abs(RandomAmount); ++i)
	{
		int32 NewType = AsteroidFieldStream.RandHelper(AsteroidTypes.Num());

		FTransform NewTransform = FTransform(
			AsteroidFieldStream.VRand().Rotation(),
			AsteroidFieldStream.VRand() * AsteroidFieldStream.FRandRange(BeltMinDistance, MaxDistance) + InStartLocation,
			FVector(AsteroidFieldStream.FRandRange(0.25f, 2.5f) * ScaleMultiplier));

		if (AsteroidTypes.IsValidIndex(NewType)) AsteroidTypes[NewType]->AddInstance(NewTransform);
	}
}

void AProceduralAsteroids::SpawnAsteroidsBox()
{
	int32 RandomAmount = AsteroidFieldStream.FRandRange(MinAmount, MaxAmount);
	for (int32 i = 0; i < FMath::Abs(RandomAmount); ++i)
	{
		int32 NewType = AsteroidFieldStream.RandHelper(AsteroidTypes.Num());

		AsteroidFieldStream.FRandRange(-MaxDistance, MaxDistance);

		FTransform NewTransform = FTransform(
			AsteroidFieldStream.VRand().Rotation(),
			FVector(AsteroidFieldStream.FRandRange(-MaxDistance, MaxDistance), AsteroidFieldStream.FRandRange(-MaxDistance, MaxDistance), AsteroidFieldStream.FRandRange(-BoxMaxDepth, BoxMaxDepth)) + this->GetActorLocation(),
			FVector(AsteroidFieldStream.FRandRange(0.25f, 2.5f) * ScaleMultiplier));

		if (AsteroidTypes.IsValidIndex(NewType)) AsteroidTypes[NewType]->AddInstance(NewTransform);
	}
}