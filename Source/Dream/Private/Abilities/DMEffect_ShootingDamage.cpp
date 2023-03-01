// Fill out your copyright notice in the Description page of Project Settings.


#include "DMEffect_ShootingDamage.h"
#include "DMAttributeSet.h"
#include "DreamDamageCalculation.h"

UDMEffect_ShootingDamage::UDMEffect_ShootingDamage()
{
    FGameplayEffectExecutionDefinition ExecDef;
    ExecDef.CalculationClass = UDreamDamageCalculation::StaticClass();
    Executions.Add(ExecDef);
    
    
    /*FGameplayEffectCue Cue;
    Cue.GameplayCueTags = FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.BaseDamage")));
    Cue.MagnitudeAttribute = DreamAttrStatics().DamageProperty;
    GameplayCues.Add(Cue);*/
}
