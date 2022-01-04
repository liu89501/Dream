// Fill out your copyright notice in the Description page of Project Settings.


#include "DAIGeneratorMultiple.h"
#include "DEnemyBase.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ADAIGeneratorMultiple::ADAIGeneratorMultiple()
{
	PrimaryActorTick.bCanEverTick = false;
	GeneratePath = CreateDefaultSubobject<USplineComponent>(TEXT("Path"));
	GeneratePath->SetupAttachment(RootComponent);
}

void ADAIGeneratorMultiple::ProcessAIGenerate()
{
	int32 Index = 0;

	int32 NumberOfSplinePoints = GeneratePath->GetNumberOfSplinePoints();
	for (TPair<TSubclassOf<ADEnemyBase>, int32> Pair : GenerateAIClass)
	{
		for (int32 N = 0; N < Pair.Value; N++)
		{
			FVector Location = GeneratePath->GetLocationAtSplinePoint(Index % NumberOfSplinePoints, ESplineCoordinateSpace::World);
			
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			ADEnemyBase* Enemy = GetWorld()->SpawnActor<ADEnemyBase>(Pair.Key, Location, FRotator::ZeroRotator, SpawnParameters);
			Enemy->SetAIGenerator(this);

			Index++;
		}
	}

	ActiveAICounter = Index;;
}
