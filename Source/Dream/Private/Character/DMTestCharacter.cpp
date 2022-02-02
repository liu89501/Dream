// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DMTestCharacter.h"

// Sets default values
ADMTestCharacter::ADMTestCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADMTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADMTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADMTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FGenericTeamId ADMTestCharacter::GetGenericTeamId() const
{
	return TeamId;
}

