// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	/**
	* 	应用GameplayEffect 到 GameplayEvent中的目标Actors
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability")
    bool ApplyGameplayEffectToAllActors(const FGameplayEventData& EventData, TSubclassOf<class UGameplayEffect> EffectClass);
};
