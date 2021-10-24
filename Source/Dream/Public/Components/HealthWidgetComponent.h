// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ScalableWidgetComponent.h"
#include "HealthWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UHealthWidgetComponent : public UScalableWidgetComponent
{
	GENERATED_BODY()

public:

	UHealthWidgetComponent();

    void UpdateStatus(float HealthPercentage) const;
};
