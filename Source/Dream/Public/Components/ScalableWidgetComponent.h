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

	/** 超过此距离将不显示 为0则永远都显示 */
	UPROPERTY(EditAnywhere, Category=Scalable)
	float HiddenDistance;

public:

	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	UPROPERTY()
	APawn* LocalPlayerCharacter;
	
};
