// Fill out your copyright notice in the Description page of Project Settings.


#include "DRepVelocityProjectileActor.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ADRepVelocityProjectileActor::ADRepVelocityProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

// Called when the game starts or when spawned
void ADRepVelocityProjectileActor::BeginPlay()
{
	Super::BeginPlay();
}

void ADRepVelocityProjectileActor::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	Super::PostNetReceiveVelocity(NewVelocity);
	ProjectileMovement->Velocity = NewVelocity;
}

// Called every frame
void ADRepVelocityProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

