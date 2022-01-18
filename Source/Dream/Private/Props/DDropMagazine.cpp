// Fill out your copyright notice in the Description page of Project Settings.

#include "DDropMagazine.h"

// Sets default values
ADDropMagazine::ADDropMagazine()
{
}

void ADDropMagazine::BeginPlay()
{
	Super::BeginPlay();

	ApplyImpulse(Mesh);
}
