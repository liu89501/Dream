// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamDamageCalculationFixed.h"
#include "DCharacterBase.h"
#include "DGameplayTags.h"
#include "DGE_DamageHealthSteal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/DreamAttributeSet.h"

UDreamDamageCalculationFixed::UDreamDamageCalculationFixed()
{
    RelevantAttributesToCapture.Add(DreamAttrStatics().DefensePowerDef);
    ValidTransientAggregatorIdentifiers.AddTag(CustomizeTags().Exec_Temporary_PercentageDmgInc);
    ValidTransientAggregatorIdentifiers.AddTag(CustomizeTags().Exec_Temporary_FixedDamage);
}

void UDreamDamageCalculationFixed::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                     FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    UAbilitySystemComponent* TargetAbilitySystem = ExecutionParams.GetTargetAbilitySystemComponent();
    UAbilitySystemComponent* SourceAbilitySystem = ExecutionParams.GetSourceAbilitySystemComponent();
    
    if (TargetAbilitySystem == SourceAbilitySystem)
    {
        return;
    }

    AActor* SourceActor = SourceAbilitySystem ? SourceAbilitySystem->GetAvatarActor() : nullptr;
    AActor* TargetActor = TargetAbilitySystem ? TargetAbilitySystem->GetAvatarActor() : nullptr;

    if (FGenericTeamId::GetAttitude(SourceActor, TargetActor) != ETeamAttitude::Hostile)
    {
        return;
    }

    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    // Gather the tags from the source and target as that can affect which buffs should be used
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    // --------------------------------------
    //	Damage Done = Damage * AttackPower / DefensePower
    //	If DefensePower is 0, it is treated as 1.0
    // --------------------------------------

    float TargetDefensePower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().DefensePowerDef, EvaluationParameters, TargetDefensePower);

    float PercentageDmgInc = 0.f;
    ExecutionParams.AttemptCalculateTransientAggregatorMagnitude(CustomizeTags().Exec_Temporary_PercentageDmgInc, EvaluationParameters, PercentageDmgInc);
    
    float FixedDamage = 0.f;
    ExecutionParams.AttemptCalculateTransientAggregatorMagnitude(CustomizeTags().Exec_Temporary_FixedDamage, EvaluationParameters, FixedDamage);

    int32 TargetLevel = 1;
    
    if (ADCharacterBase* TargetCharacter = Cast<ADCharacterBase>(TargetActor))
    {
        TargetLevel = TargetCharacter->GetCharacterLevel();
    }

    float DamageDone = FixedDamage * (1 + PercentageDmgInc) * (1 - TargetDefensePower / (TargetDefensePower + 1500 - 20 * TargetLevel));
    
    DamageDone = FMath::Max(DamageDone, 1.f);

    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DreamAttrStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
}
