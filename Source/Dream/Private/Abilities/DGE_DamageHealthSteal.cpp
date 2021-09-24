// Fill out your copyright notice in the Description page of Project Settings.


#include "DGE_DamageHealthSteal.h"
#include "DreamAttributeSet.h"

FGameplayTag UDGE_DamageHealthSteal::HealthStealSetByCallerTag = FGameplayTag::RequestGameplayTag(TEXT("GE.SetByCaller.HealthSteal"));

UDGE_DamageHealthSteal::UDGE_DamageHealthSteal()
{
    FSetByCallerFloat ModifierMagnitudeHealth;
    ModifierMagnitudeHealth.DataTag = HealthStealSetByCallerTag;
    
    FGameplayModifierInfo ModifierHealth;
    ModifierHealth.Attribute = DreamAttrStatics().HealthStealProperty;
    ModifierHealth.ModifierMagnitude = ModifierMagnitudeHealth;
    ModifierHealth.ModifierOp = EGameplayModOp::Additive;

    Modifiers.Add(ModifierHealth);
}
