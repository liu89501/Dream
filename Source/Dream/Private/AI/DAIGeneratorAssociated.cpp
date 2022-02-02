// Fill out your copyright notice in the Description page of Project Settings.

#include "DAIGeneratorAssociated.h"
#include "BroadcastReceiverComponent.h"
#include "DAIGeneratorUnit.h"
#include "DEnemyBase.h"
#include "NavigationSystem.h"

// Sets default values
ADAIGeneratorAssociated::ADAIGeneratorAssociated()
{
	PrimaryActorTick.bCanEverTick = false;
	BroadcastReceiver = CreateDefaultSubobject<UBroadcastReceiverComponent>(TEXT("BroadcastReceiver"));
}

int32 ADAIGeneratorAssociated::ProcessAIGenerate()
{
	for (ADAIGeneratorUnit* Unit : AIInstance)
	{
		if (!Unit)
		{
			UE_LOG(LogDream, Error, TEXT("GeneratorUnit Invalid"));
			continue;
		}
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ADEnemyBase* Enemy = GetWorld()->SpawnActor<ADEnemyBase>(Unit->AIClass, Unit->GetActorTransform(), SpawnParameters);
		
		Enemy->SetAIGenerator(this);

		Unit->InitializeAI(Enemy);
	}

	return AIInstance.Num();
}

void ADAIGeneratorAssociated::Destroyed()
{
	Super::Destroyed();
	
	for (ADAIGeneratorUnit* Unit : AIInstance)
	{
		if (Unit)
		{
			Unit->Destroy();
		}
	}
}

#if WITH_EDITOR

void ADAIGeneratorAssociated::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property == nullptr)
	{
		return;
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ADAIGeneratorAssociated, AIInstance))
	{
		for (ADAIGeneratorUnit*& Unit : AIInstance)
		{
			if (Unit == nullptr)
			{
				if (UWorld* World = GetWorld())
				{
					Unit = World->SpawnActor<ADAIGeneratorUnit>(GetActorLocation() + FVector(100.f), FRotator::ZeroRotator);
				}
			}
		}
	}
}

#endif