// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameplayPerk.h"
#include "AbilitySystemComponent.h"

UDreamGameplayPerk::UDreamGameplayPerk()
{
}

FActiveGameplayEffectHandle UDreamGameplayPerk::ActivationPerkEffect_Implementation(UAbilitySystemComponent* TargetAbilitySystem)
{
    if (PerkEffect && TargetAbilitySystem)
    {
        UGameplayEffect* GameplayEffect = PerkEffect->GetDefaultObject<UGameplayEffect>();
        FGameplayEffectContextHandle EffectContext = TargetAbilitySystem->MakeEffectContext();
        return TargetAbilitySystem->ApplyGameplayEffectToSelf(GameplayEffect, INDEX_NONE, EffectContext);
    }

    return FActiveGameplayEffectHandle();
}

