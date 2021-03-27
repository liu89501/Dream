// Fill out your copyright notice in the Description page of Project Settings.


#include "DGE_WeaponBaseDamage.h"

#include "DreamAttributeSet.h"
#include "DreamDamageCalculation.h"

UDGE_WeaponBaseDamage::UDGE_WeaponBaseDamage()
{
    FGameplayEffectExecutionDefinition ExecDef;
    ExecDef.CalculationClass = UDreamDamageCalculation::StaticClass();
    Executions.Add(ExecDef);
    
    /*FGameplayEffectCue Cue;
    Cue.GameplayCueTags = FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.BaseDamage")));
    Cue.MagnitudeAttribute = DreamAttrStatics().DamageProperty;
    GameplayCues.Add(Cue);*/
}
