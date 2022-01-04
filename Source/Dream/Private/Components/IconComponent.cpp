// Fill out your copyright notice in the Description page of Project Settings.


#include "IconComponent.h"

// Sets default values for this component's properties
UIconComponent::UIconComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bAutoActivate = false;
	// ...
}

// Called when the game starts
void UIconComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UIconComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

