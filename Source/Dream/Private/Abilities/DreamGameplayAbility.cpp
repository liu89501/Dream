// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DreamType.h"

bool UDreamGameplayAbility::ApplyGameplayEffectToAllActors(const FGameplayEventData& EventData, TSubclassOf<UGameplayEffect> EffectClass)
{
    if (GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
    {
        DREAM_NLOG(Error, TEXT("Ability Invalid"));
        return false;
    }

    if (EventData.TargetData.Data.Num() == 0)
    {
        DREAM_NLOG(Error, TEXT("EventData.TargetData Is Empty"));
        return false;
    }

    TSharedPtr<FGameplayAbilityTargetData> TargetData = EventData.TargetData.Data[0];

    if (!TargetData.IsValid())
    {
        DREAM_NLOG(Error, TEXT("TargetData Invalid"));
        return false;
    }

    UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();

    TArray<TWeakObjectPtr<AActor>> TargetActors = TargetData->GetActors();
    if (TargetActors.Num() == 0)
    {
        DREAM_NLOG(Verbose, TEXT("TargetActors Is Empty"));
        return true;
    }

    for (TWeakObjectPtr<AActor> Target : TargetActors)
    {
        if (!Target.IsValid())
        {
            continue;
        }

        UAbilitySystemComponent* TargetComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target.Get());
        if (TargetComponent == nullptr)
        {
            continue;
        }
		
        FGameplayEffectContextHandle EffectContextHandle = AbilitySystem->MakeEffectContext();
        EffectContextHandle.SetAbility(this);
        EffectContextHandle.AddOrigin(TargetData->GetOrigin().GetLocation());
        TargetComponent->ApplyGameplayEffectToSelf(
            EffectClass->GetDefaultObject<UGameplayEffect>(), GetAbilityLevel(), EffectContextHandle);
    }

    return true;
}
