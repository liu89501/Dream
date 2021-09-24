// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DNPCCharacter.h"

// Sets default values
ADNPCCharacter::ADNPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MiniMapDataComp = CreateDefaultSubobject<UMiniMapDataComponent>(TEXT("MiniMapDataComp"));
	MiniMapDataComp->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void ADNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UMiniMapDataComponent* ADNPCCharacter::GetMiniMapDataComponent() const
{
	return MiniMapDataComp;
}

