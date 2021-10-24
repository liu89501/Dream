// Fill out your copyright notice in the Description page of Project Settings.


#include "DAIGeneratorSingle.h"
#include "DEnemyBase.h"
#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ADAIGeneratorSingle::ADAIGeneratorSingle()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADAIGeneratorSingle::ProcessAIGenerate()
{
	ActiveAICounter.Set(1);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ADEnemyBase* Enemy = GetWorld()->SpawnActor<ADEnemyBase>(GenerateAIClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParameters);
	Enemy->SetAIGenerator(this);
}
