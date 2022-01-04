// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DreamDamageCalculationFixed.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDreamDamageCalculationFixed : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

    UDreamDamageCalculationFixed();

public:

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
