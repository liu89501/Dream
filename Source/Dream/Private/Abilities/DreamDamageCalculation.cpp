// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/DreamDamageCalculation.h"


#include "DCharacterBase.h"
#include "DGameplayTags.h"
#include "DGE_DamageHealthSteal.h"
#include "DreamGameplayType.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/DreamAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"

UDreamDamageCalculation::UDreamDamageCalculation()
{
    RelevantAttributesToCapture.Add(DreamAttrStatics().DefensePowerDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().DamageDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().CriticalRateDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().CriticalDamageDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().IncreaseAtkPowPercentageDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().HealthStealPercentageDef);
}

void UDreamDamageCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
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

    float DefensePower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().DefensePowerDef, EvaluationParameters, DefensePower);

    float AttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().AttackPowerDef, EvaluationParameters, AttackPower);

    float IncreaseAtkPowPercentage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().IncreaseAtkPowPercentageDef, EvaluationParameters, IncreaseAtkPowPercentage);

    float Damage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().DamageDef, EvaluationParameters, Damage);

    FDreamGameplayEffectContext* DreamEffectContext = nullptr;
    
    if (FGameplayEffectContext* EffectContext = Spec.GetContext().Get())
    {
        if (EffectContext->GetScriptStruct() == FDreamGameplayEffectContext::StaticStruct())
        {
            DreamEffectContext = StaticCast<FDreamGameplayEffectContext*>(EffectContext);
        }
    }

    float CriticalDamageAddition = 0.f;
    float DamageFalloffPercentage = 0.f;
    float IncreaseDamagePercentage = 0.f;

    if (DreamEffectContext)
    {
        // 暴击相关计算
        float CriticalRate = 0.f;
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().CriticalRateDef, EvaluationParameters, CriticalRate);

        if (UKismetMathLibrary::RandomBoolWithWeight(CriticalRate))
        {
            DreamEffectContext->SetDamageCritical(true);
            ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().CriticalDamageDef, EvaluationParameters, CriticalDamageAddition);
        }
        
        // 伤害距离衰减
        DamageFalloffPercentage = 1.f - DreamEffectContext->GetDamageFalloffPercentage();

        // 弱点增伤
        IncreaseDamagePercentage = DreamEffectContext->GetWeakPointIncreaseDamagePercentage();

        // 造成得伤害 * 弹片数量
        //DamageDone *= DreamEffectContext->GetHitPoints().Num();
    }

    int32 CharacterLevel = 1;
    if (ADCharacterBase* TargetCharacter = Cast<ADCharacterBase>(TargetActor))
    {
        CharacterLevel = TargetCharacter->GetCharacterLevel();
    }

    float DamageDone = (AttackPower + Damage)
        * (1 + IncreaseAtkPowPercentage + IncreaseDamagePercentage)
        * (1 + CriticalDamageAddition)
        * (1 - DamageFalloffPercentage - DefensePower / (DefensePower + 1500 - 20 * CharacterLevel));
    
    DamageDone = FMath::Max(DamageDone, 1.f);

    // 触发 GE.Executions.ConditionalGameplayEffects 
    OutExecutionOutput.MarkConditionalGameplayEffectsToTrigger();

    // 生命偷取计算
    float HealthStealPercentage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().HealthStealPercentageDef, EvaluationParameters, HealthStealPercentage);
    
    // 对源应用回血效果
    float TreatmentAmount = HealthStealPercentage * DamageDone;
    if (TreatmentAmount > 1.f)
    {
        FGameplayEffectSpec TreatmentEffectSpec(GetDefault<UDGE_DamageHealthSteal>(), SourceAbilitySystem->MakeEffectContext());
        TreatmentEffectSpec.SetSetByCallerMagnitude(CustomizeTags().HealthStealSetByCallerTag, TreatmentAmount);
        SourceAbilitySystem->ApplyGameplayEffectSpecToSelf(TreatmentEffectSpec);
    }

    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DreamAttrStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
}
