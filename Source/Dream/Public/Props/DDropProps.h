// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "GameFramework/Actor.h"
#include "DDropProps.generated.h"

UCLASS(Abstract)
class DREAM_API ADDropProps : public AActor
{
	GENERATED_BODY()

public:

	ADDropProps();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=DropProps)
	FRangeRandomFloat RandomImpulseXY;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=DropProps)
	float ImpulseZ;

protected:

	void ApplyImpulse(UMeshComponent* MeshComponent);

};
