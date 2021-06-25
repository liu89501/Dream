// Fill out your copyright notice in the Description page of Project Settings.


#include "DGE_DamageHealthSteal.h"

#include "DreamAttributeSet.h"

FGameplayTag UDGE_DamageHealthSteal::HSHealthSetByCallerTag = FGameplayTag::RequestGameplayTag(TEXT("GE.SetByCaller.HealthSteal.Health"));
FGameplayTag UDGE_DamageHealthSteal::HSShieldSetByCallerTag = FGameplayTag::RequestGameplayTag(TEXT("GE.SetByCaller.HealthSteal.Shield"));

UDGE_DamageHealthSteal::UDGE_DamageHealthSteal()
{
    FSetByCallerFloat ModifierMagnitudeHealth;
    ModifierMagnitudeHealth.DataTag = HSHealthSetByCallerTag;
    FSetByCallerFloat ModifierMagnitudeShield;
    ModifierMagnitudeShield.DataTag = HSShieldSetByCallerTag;
    
    FGameplayModifierInfo ModifierHealth;
    ModifierHealth.Attribute = DreamAttrStatics().HealthProperty;
    ModifierHealth.ModifierMagnitude = ModifierMagnitudeHealth;
    ModifierHealth.ModifierOp = EGameplayModOp::Additive;

    FGameplayModifierInfo ModifierShield;
    ModifierShield.Attribute = DreamAttrStatics().ShieldProperty;
    ModifierShield.ModifierMagnitude = ModifierMagnitudeShield;
    ModifierShield.ModifierOp = EGameplayModOp::Additive;

    Modifiers.Add(ModifierHealth);
    Modifiers.Add(ModifierShield);
}
