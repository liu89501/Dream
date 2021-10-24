// Fill out your copyright notice in the Description page of Project Settings.


#include "DAIGeneratorBase.h"
#include "NavigationSystem.h"
#include "Components/BillboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ADAIGeneratorBase::ADAIGeneratorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ResetWaitTime = 10.f;

	CurrentRevivedCount = 0.f;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icon"));
	RootComponent = Billboard;
}

void ADAIGeneratorBase::ActivateAIGenerator()
{
	if (GetNetMode() <= NM_DedicatedServer)
	{
		ProcessAIGenerate();
	}
}

void ADAIGeneratorBase::AIDeathCount()
{
	// 服务器或单机模式下才能执行后面的逻辑
	if (GetNetMode() > NM_DedicatedServer)
	{
		return;
	}
	
	if (ActiveAICounter.Decrement() == 0)
	{
		if (CurrentRevivedCount < ReviveFrequency || ReviveFrequency == -1)
		{
			CurrentRevivedCount++;
			FTimerHandle Handle;
			GetWorldTimerManager().SetTimer(Handle, this, &ADAIGeneratorBase::ProcessAIGenerate, ResetWaitTime);
		}
		else
		{
			OnAllAIDestroyed.Broadcast(this);
		}
	}
}

// Called when the game starts or when spawned
void ADAIGeneratorBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() > NM_DedicatedServer)
	{
		Destroy();
	}

	if (bAutoActive)
	{
		ActivateAIGenerator();
	}
}

void ADAIGeneratorBase::ProcessAIGenerate()
{
}

