// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamGameplayAbility.h"
#include "DAbility_WeaponFire.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDAbility_WeaponFire : public UGameplayAbility
{
    GENERATED_BODY()

public:

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                            const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo,
                            bool bReplicateEndAbility,
                            bool bWasCancelled) override;

protected:

    virtual void OnTriggerEvent(const FGameplayEventData* EventData);

private:

    FDelegateHandle DelegateHandle;
};
