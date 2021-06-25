// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DGameplayEffectUIData.h"
#include "Abilities/GameplayAbility.h"
#include "DreamGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDreamGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, Instanced, Category=DreamAbility)
	UDGameplayEffectUIData* AbilityUIData;
};
