// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralAsteroids.generated.h"

UCLASS()
class GAMEJAM_API AProceduralAsteroids : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralAsteroids();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	bool bBoxField = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	int32 CurrentSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	int32 MinAmount = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	int32 MaxAmount = 350;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	int32 BeltLength = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float BeltPointDistance = 100000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float BeltMinDistance = 20000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float MaxDistance = 100000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float BoxMaxDepth = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float ScaleMultiplier = 1.f;

private:
	UInstancedStaticMeshComponent* AsteroidA = nullptr;
	UInstancedStaticMeshComponent* AsteroidB = nullptr;
	UInstancedStaticMeshComponent* AsteroidC = nullptr;
	UInstancedStaticMeshComponent* AsteroidD = nullptr;
	UInstancedStaticMeshComponent* AsteroidE = nullptr;
	TArray<UInstancedStaticMeshComponent*> AsteroidTypes;

	FRandomStream AsteroidFieldStream;

	FVector BeltDirection = FVector(1.f, 0.f, 0.f);
	FVector LastPoint = FVector(0.f, 0.f, 0.f);

	void GenerateBeltPath(int32 InSeed);
	FVector GenerateNextBeltPoint(FVector InDirection);
	void SpawnAsteroidInstances(FVector InStartLocation);

	void SpawnAsteroidsBox();
};
