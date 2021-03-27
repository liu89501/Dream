// Fill out your copyright notice in the Description page of Project Settings.


#include "DAIGenerator.h"
#include "DreamType.h"
#include "DEnemyBase.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ADAIGenerator::ADAIGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	ResetWaitTime = 10.f;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	RootComponent = Billboard;

	GeneratorRangeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("GeneratorRangeBox"));
	GeneratorRangeBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GeneratorRangeBox->SetupAttachment(RootComponent);
}

void ADAIGenerator::AIDeathCount()
{
	if (ActiveAICounter.Decrement() == 0)
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, this, &ADAIGenerator::ResetGenerator, ResetWaitTime);
	}
}

// Called when the game starts or when spawned
void ADAIGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() <= NM_DedicatedServer)
	{
		ResetGenerator();
	}
	else
	{
		Destroy();
	}
}

void ADAIGenerator::ResetGenerator()
{
	ActiveAICounter.Set(GenerateAIClass.Num());

	for (TSubclassOf<ADEnemyBase> AIClass : GenerateAIClass)
	{
		FVector Origin = GeneratorRangeBox->GetComponentLocation();
		FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, GeneratorRangeBox->GetScaledBoxExtent());
		RandomPoint.Z = Origin.Z;
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ADEnemyBase* Enemy = GetWorld()->SpawnActor<ADEnemyBase>(AIClass, RandomPoint, FRotator::ZeroRotator, SpawnParameters);
		Enemy->SetAIGenerator(this);
	}
}

