// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DreamDamageCalculationFixed.generated.h"

/**
 * 固定伤害，攻击力不会计算
 */
UCLASS()
class DREAM_API UDreamDamageCalculationFixed : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UDreamDamageCalculationFixed();

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
