// Fill out your copyright notice in the Description page of Project Settings.


#include "DItemPreview.h"
#include "DCharacterPlayer.h"
#include "Components/DirectionalLightComponent.h"

// Sets default values
ADItemPreview::ADItemPreview()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = Scene;
}

// Called when the game starts or when spawned
void ADItemPreview::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADItemPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

