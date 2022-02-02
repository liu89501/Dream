// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DreamDamageCalculation.generated.h"

enum class EDDamageType : uint8;

/**
 * 
 */
UCLASS()
class DREAM_API UDreamDamageCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UDreamDamageCalculation();

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;


protected:

	bool GetAttributeCaptureFromDamageType(EDDamageType DamageType, FGameplayEffectAttributeCaptureDefinition& OutDef) const;
};
