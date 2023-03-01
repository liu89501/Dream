// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DMAbility_Shooting.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDMAbility_Shooting : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UDMAbility_Shooting();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
};
