// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/DreamDamageCalculation.h"
#include "DCharacterBase.h"
#include "DGameplayTags.h"
#include "DGE_DamageHealthSteal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/DreamAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"

UDreamDamageCalculation::UDreamDamageCalculation()
{
    RelevantAttributesToCapture.Add(DreamAttrStatics().DefensePowerDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().CriticalRateDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().CriticalDamageDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().HealthStealDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().WeaponDamageAssaultRifleDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().WeaponDamageGrenadeLaunchDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().WeaponDamageShotgunDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().WeaponDamageSniperRifleDef);
    RelevantAttributesToCapture.Add(DreamAttrStatics().WeaponDamagePrecisionRifleDef);

    ValidTransientAggregatorIdentifiers.AddTag(CustomizeTags().Exec_Temporary_PercentageDmgInc);
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

    float PercentageDmgInc = 0.f;
    ExecutionParams.AttemptCalculateTransientAggregatorMagnitude(CustomizeTags().Exec_Temporary_PercentageDmgInc, EvaluationParameters, PercentageDmgInc);

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
    float DamageTypeIncPercentage = 0.f;

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
        
        // 距离伤害衰减
        DamageFalloffPercentage = 1.f - DreamEffectContext->GetDamageFalloffPercentage();

        // 弱点增伤
        IncreaseDamagePercentage = DreamEffectContext->GetWeakPointIncreaseDamagePercentage();

        // 武器类型增伤
        FGameplayEffectAttributeCaptureDefinition CaptureDef;
        if (GetAttributeCaptureFromDamageType(DreamEffectContext->GetDamageType(), CaptureDef))
        {
            ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, DamageTypeIncPercentage);
        }

        // 造成得伤害 * 弹片数量
        //DamageDone *= DreamEffectContext->GetHitPoints().Num();
    }

    int32 CharacterLevel = 1;
    if (ADCharacterBase* TargetCharacter = Cast<ADCharacterBase>(TargetActor))
    {
        CharacterLevel = TargetCharacter->GetCharacterLevel();
    }

    float DamageDone = AttackPower
        * (1 + PercentageDmgInc + IncreaseDamagePercentage + CriticalDamageAddition)
        * (1 + DamageTypeIncPercentage)
        * (1 - DamageFalloffPercentage - DefensePower / (DefensePower + 1500 - 20 * CharacterLevel));
    
    DamageDone = FMath::Max(DamageDone, 1.f);

    // 触发 GE.Executions.ConditionalGameplayEffects 
    OutExecutionOutput.MarkConditionalGameplayEffectsToTrigger();

    // 生命偷取计算
    float HealthStealPercentage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DreamAttrStatics().HealthStealDef, EvaluationParameters, HealthStealPercentage);
    
    // 对源应用回血效果
    float TreatmentAmount = HealthStealPercentage * DamageDone;
    if (TreatmentAmount > 1.f)
    {
        FGameplayEffectSpec TreatmentEffectSpec(GetDefault<UDGE_DamageHealthSteal>(), SourceAbilitySystem->MakeEffectContext());
        TreatmentEffectSpec.SetSetByCallerMagnitude(CustomizeTags().SetByCaller_HealthSteal, TreatmentAmount);
        SourceAbilitySystem->ApplyGameplayEffectSpecToSelf(TreatmentEffectSpec);
    }

    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DreamAttrStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
}

bool UDreamDamageCalculation::GetAttributeCaptureFromDamageType(EDDamageType DamageType, FGameplayEffectAttributeCaptureDefinition& OutDef) const
{
    bool bValid = true;
    
    switch (DamageType)
    {
    case EDDamageType::Weapon_Shotgun:

        OutDef = DreamAttrStatics().WeaponDamageShotgunDef;
        break;

    case EDDamageType::Weapon_AssaultRifle:
            
        OutDef = DreamAttrStatics().WeaponDamageAssaultRifleDef;
        break;

    case EDDamageType::Weapon_GrenadeLaunch:

        OutDef = DreamAttrStatics().WeaponDamageGrenadeLaunchDef;
        break;
        
    case EDDamageType::Weapon_PrecisionRifle:

        OutDef = DreamAttrStatics().WeaponDamagePrecisionRifleDef;
        break;
        
    case EDDamageType::Weapon_SniperRifle:

        OutDef = DreamAttrStatics().WeaponDamageSniperRifleDef;
        break;
        
    default:
        bValid = false;
    }

    return bValid;
}
