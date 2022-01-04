// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DNPCCharacter.h"

// Sets default values
ADNPCCharacter::ADNPCCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	IconComponent = CreateDefaultSubobject<UIconComponent>(TEXT("IconComponent"));
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

UIconComponent* ADNPCCharacter::GetIconComponent() const
{
	return IconComponent;
}

