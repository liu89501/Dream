// Fill out your copyright notice in the Description page of Project Settings.


#include "DGE_DamageHealthSteal.h"

#include "DreamAttributeSet.h"

FGameplayTag UDGE_DamageHealthSteal::HealthStealSetByCallerTag = FGameplayTag::RequestGameplayTag(TEXT("GE.SetByCaller.HealthSteal"));

UDGE_DamageHealthSteal::UDGE_DamageHealthSteal()
{
    FGameplayModifierInfo ModifierInfo;
    ModifierInfo.Attribute = DreamAttrStatics().HealthProperty;

    FSetByCallerFloat ModifierMagnitude;
    ModifierMagnitude.DataTag = HealthStealSetByCallerTag;
    ModifierInfo.ModifierMagnitude = ModifierMagnitude;
    ModifierInfo.ModifierOp = EGameplayModOp::Additive;

    /*FGameplayEffectCue Cue;
    Cue.GameplayCueTags = FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.HealthSteal")));
    Cue.MagnitudeAttribute = DreamAttrStatics().HealthProperty;
    GameplayCues.Add(Cue);*/

    Modifiers.Add(ModifierInfo);
}
