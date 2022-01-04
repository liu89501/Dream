// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PreviewSceneSettings.generated.h"

/**
 * 
 */
UCLASS(config=Engine, defaultconfig)
class DREAM_API UPreviewSceneSettings : public UObject
{
	GENERATED_BODY()

	UPreviewSceneSettings();

public:

	UPROPERTY(Config, EditAnywhere, Category=Light)
	bool bUseDirectionalLight;

	UPROPERTY(Config, EditAnywhere, meta=(EditCondition="bUseDirectionalLight"), Category=Light)
	float LightBrightness;

	UPROPERTY(Config, EditAnywhere, meta=(HideAlphaChannel, EditCondition="bUseDirectionalLight"), Category=Light)
	FColor LightColor;
	
	UPROPERTY(Config, EditAnywhere, meta=(EditCondition="bUseDirectionalLight"), Category=Light)
	FRotator LightRotation;

	UPROPERTY(Config, EditAnywhere, Category=Light)
	bool bUseSkyLight;

	UPROPERTY(Config, EditAnywhere, meta=(EditCondition="bUseSkyLight"), Category=Light)
	float SkyLightIntensity;
	
	UPROPERTY(Config, EditAnywhere, meta=(EditCondition="bUseSkyLight"), Category=Light)
	float SkyDistanceThreshold;

	UPROPERTY(Config, EditAnywhere, meta=(MetaClass="DPreviewScene"), Category=PreviewScene)
	FSoftClassPath PreviewSceneClass;

public:

	static UPreviewSceneSettings* Get();
	
};
