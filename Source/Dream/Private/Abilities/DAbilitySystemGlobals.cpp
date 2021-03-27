// Fill out your copyright notice in the Description page of Project Settings.


#include "DAbilitySystemGlobals.h"
#include "DreamGameplayType.h"

FGameplayEffectContext* UDAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FDreamGameplayEffectContext();
}
