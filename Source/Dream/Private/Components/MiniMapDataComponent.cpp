// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniMapDataComponent.h"

// Sets default values for this component's properties
UMiniMapDataComponent::UMiniMapDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


void UMiniMapDataComponent::CalculationPosition(const FVector& TargetLocation, const FRotator& TargetRotation, float ScanRange, FMiniMapData& OutData)
{
	FVector Location = GetOwner()->GetActorLocation();
	float Distance = FVector2D::Distance(FVector2D(TargetLocation), FVector2D(Location));

	FRotator LocalRotation = (Location - TargetLocation).Rotation() - TargetRotation;
	LocalRotation.Pitch = 0;
	LocalRotation.Normalize();

	OutData.Yaw = bFixedSpriteDirection ? 0.f : LocalRotation.Yaw;
	OutData.PosNormalize = FVector2D(LocalRotation.Vector().GetSafeNormal());
	OutData.DistancePercentage = Distance / ScanRange;
	OutData.DrawType = DrawType;
	OutData.SpriteBrush = &DefaultSprite;
	OutData.OverflowSpriteBrush = &OverflowSprite;
}

// Called when the game starts
void UMiniMapDataComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMiniMapDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

