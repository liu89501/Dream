// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "Engine/DataAsset.h"
#include "SurfaceImpactAsset.generated.h"


/**
 * 
 */
UCLASS()
class DREAM_API USurfaceImpactAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = ProjectSettings)
	TMap<TEnumAsByte<EPhysicalSurface>, FSurfaceImpactEffect> SurfaceImpactEffects;
};
