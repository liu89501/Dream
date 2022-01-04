// Fill out your copyright notice in the Description page of Project Settings.


#include "DPreviewScene.h"

// Sets default values
ADPreviewScene::ADPreviewScene()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>("Scene");
	RootComponent = Scene;
	
	PreviewActorPoint = CreateDefaultSubobject<USceneComponent>("PreviewActorPoint");
	PreviewActorPoint->SetupAttachment(RootComponent);
	
	SceneActor = CreateDefaultSubobject<USceneComponent>("SceneActor");
	SceneActor->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADPreviewScene::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADPreviewScene::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

