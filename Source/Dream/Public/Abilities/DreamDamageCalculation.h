// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamGameplayType.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DreamDamageCalculation.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDreamDamageCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

    UDreamDamageCalculation();

public:

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;


protected:

	bool GetAttributeCaptureFromDamageType(EDDamageType DamageType, FGameplayEffectAttributeCaptureDefinition& OutDef) const;
};
