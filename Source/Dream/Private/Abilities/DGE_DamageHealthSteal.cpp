// Fill out your copyright notice in the Description page of Project Settings.


#include "DGE_DamageHealthSteal.h"

#include "DGameplayTags.h"
#include "DMAttributeSet.h"

UDGE_DamageHealthSteal::UDGE_DamageHealthSteal()
{
    FSetByCallerFloat ModifierMagnitudeHealth;
    ModifierMagnitudeHealth.DataTag = CustomizeTags().SetByCaller_HealthSteal;
    
    FGameplayModifierInfo ModifierHealth;
    ModifierHealth.Attribute = DMAttrStatics().HealthStealProperty;
    ModifierHealth.ModifierMagnitude = ModifierMagnitudeHealth;
    ModifierHealth.ModifierOp = EGameplayModOp::Additive;

    Modifiers.Add(ModifierHealth);
}
