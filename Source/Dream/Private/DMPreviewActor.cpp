// Fill out your copyright notice in the Description page of Project Settings.

#include "DMPreviewActor.h"

// Sets default values
ADMPreviewActor::ADMPreviewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = Scene;
}

