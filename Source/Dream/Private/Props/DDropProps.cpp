﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "DDropProps.h"

// Sets default values
ADDropProps::ADDropProps()
	: RandomImpulseXY(FRangeRandomFloat(50.f, 80.f))
    , ImpulseZ(400.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ADDropProps::ApplyImpulse(UMeshComponent* MeshComponent)
{
	float RandomX = RandomImpulseXY.GetRandomFloat();
	float RandomY = RandomImpulseXY.GetRandomFloat();

	MeshComponent->AddImpulse(FVector(RandomX, RandomY, ImpulseZ), NAME_None, true);
}

