// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewSceneSettings.h"

UPreviewSceneSettings::UPreviewSceneSettings()
        : bUseDirectionalLight(true),
		  LightBrightness(1.f),
		  LightColor(FColor::White),
          LightRotation(FRotator::ZeroRotator),
		  bUseSkyLight(true),
          SkyLightIntensity(1.f),
          SkyDistanceThreshold(500.f)
{
}

UPreviewSceneSettings* UPreviewSceneSettings::Get()
{
	return UPreviewSceneSettings::StaticClass()->GetDefaultObject<UPreviewSceneSettings>();
}
