// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/DreamDamageCalculation.h"

#include "DreamGameplayType.h"
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

    //AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
    //AActor* TargetActor = TargetAbilitySystem ? TargetAbilitySystem->AvatarActor : nullptr;

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
    if (DefensePower == 0.0f)
    {
        DefensePower = 1.0f;
    }

    float AttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().AttackPowerDef, EvaluationParameters, AttackPower);

    float IncreaseAtkPowPercentage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().IncreaseAtkPowPercentageDef, EvaluationParameters, IncreaseAtkPowPercentage);
    if (IncreaseAtkPowPercentage > 0.f)
    {
        AttackPower += AttackPower * IncreaseAtkPowPercentage;
    }

    float Damage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().DamageDef, EvaluationParameters, Damage);

    float DamageDone = FMath::Max(Damage, 1.f) * AttackPower / FMath::Max(DefensePower, 1.f);

    FDreamGameplayEffectContext* DreamEffectContext = nullptr;
    
    if (FGameplayEffectContext* EffectContext = Spec.GetContext().Get())
    {
        if (EffectContext->GetScriptStruct() == FDreamGameplayEffectContext::StaticStruct())
        {
            DreamEffectContext = StaticCast<FDreamGameplayEffectContext*>(EffectContext);
        }
    }

    if (DreamEffectContext)
    {
        // 暴击相关计算
        float CriticalRate = 0.f;
        if (ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().CriticalRateDef, EvaluationParameters, CriticalRate))
        {
            if (UKismetMathLibrary::RandomBoolWithWeight(FMath::Max(CriticalRate, 0.f)))
            {
                DreamEffectContext->SetDamageCritical(true);
                
                float CriticalDamage = 0.f;
                ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().CriticalDamageDef, EvaluationParameters, CriticalDamage);

                if (CriticalDamage > 0.f)
                {
                    DamageDone += DamageDone * CriticalDamage;
                }
            }
        }
        
        // 伤害距离衰减
        float DamageFalloffPercentage = DreamEffectContext->GetDamageFalloffPercentage();
        DamageDone *= DamageFalloffPercentage;

        // 弱点增伤
        float IncreaseDamagePercentage = DreamEffectContext->GetWeakPointIncreaseDamagePercentage();
        DamageDone += DamageDone * IncreaseDamagePercentage;

        // 造成得伤害 * 弹片数量
        //DamageDone *= DreamEffectContext->GetHitPoints().Num();
    }

    DamageDone = FMath::Max(DamageDone, 1.f);

    OutExecutionOutput.MarkConditionalGameplayEffectsToTrigger();

    // 生命偷取计算
    float HealthStealPercentage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().HealthStealPercentageDef, EvaluationParameters, HealthStealPercentage);
    if (HealthStealPercentage > 0)
    {
        // 对源目标应用回血效果
        float TreatmentAmount = HealthStealPercentage * DamageDone;
        if (TreatmentAmount > 1.f)
        {
            OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DreamAttrStatics().HealthStealProperty, EGameplayModOp::Additive, TreatmentAmount));
        }
    }

    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DreamAttrStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
}
