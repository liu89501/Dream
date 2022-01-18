// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ScalableWidgetComponent.h"
#include "DamageWidgetComponent.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class DREAM_API UDamageWidgetComponent : public UScalableWidgetComponent
{
	GENERATED_BODY()

	UDamageWidgetComponent();

public:

	UPROPERTY(EditAnywhere, Category=DamageWidget)
	float PendingDestroyTime;

public:

	void ActiveComponent(float Damage, bool bCritical, bool bIsHealthSteal);
	
	UFUNCTION(BlueprintImplementableEvent)
	void InitializeAttribute(float Damage, bool bCritical, bool bIsHealthSteal);

protected:

	virtual void OnDestroyDamageWidgetComponent();
	
};
