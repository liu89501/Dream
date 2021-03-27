// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "DAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:

    virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
