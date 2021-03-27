// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "ScalableWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UScalableWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UScalableWidgetComponent();

	UPROPERTY(EditAnywhere, Category=Scalable)
	float DistanceA;
	UPROPERTY(EditAnywhere, Category=Scalable)
	float DistanceB;

	UPROPERTY(EditAnywhere, Category=Scalable)
	float ScaleA;
	UPROPERTY(EditAnywhere, Category=Scalable)
	float ScaleB;

	UPROPERTY(EditAnywhere, Category=Scalable)
	bool bUseScale;

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
