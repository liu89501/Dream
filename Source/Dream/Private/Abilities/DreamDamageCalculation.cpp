// Fill out your copyright notice in the Description page of Project Settings.

#include "DreamDamageCalculation.h"
#include "DCharacterBase.h"
#include "DGameplayTags.h"
#include "DreamGameplayType.h"
#include "DGE_DamageHealthSteal.h"
#include "DMAbilitiesStatics.h"
#include "GenericTeamAgentInterface.h"
#include "DMAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"

UDreamDamageCalculation::UDreamDamageCalculation()
{
    RelevantAttributesToCapture.Add(DMAttrStatics().DefensePowerDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().CriticalRateDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().CriticalDamageDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().HealthStealDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().WeaponDamageAssaultRifleDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().WeaponDamageGrenadeLaunchDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().WeaponDamageShotgunDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().WeaponDamageSniperRifleDef);
    RelevantAttributesToCapture.Add(DMAttrStatics().WeaponDamagePrecisionRifleDef);

#if WITH_EDITORONLY_DATA

    InvalidScopedModifierAttributes.Add(DMAttrStatics().DamageDef);
    ValidTransientAggregatorIdentifiers.AddTag(CustomizeTags().Exec_Temporary_PercentageDmgInc);

#endif
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

    ADCharacterBase* SourceCharacter = SourceAbilitySystem ? Cast<ADCharacterBase>(SourceAbilitySystem->GetAvatarActor()) : nullptr;
    ADCharacterBase* TargetCharacter = TargetAbilitySystem ? Cast<ADCharacterBase>(TargetAbilitySystem->GetAvatarActor()) : nullptr;

    if (SourceCharacter == nullptr || TargetCharacter == nullptr)
    {
        return;
    }

    if (FGenericTeamId::GetAttitude(SourceCharacter, TargetCharacter) != ETeamAttitude::Hostile)
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
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DMAttrStatics().DefensePowerDef, EvaluationParameters, DefensePower);

    float AttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DMAttrStatics().AttackPowerDef, EvaluationParameters, AttackPower);

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
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DMAttrStatics().CriticalRateDef, EvaluationParameters, CriticalRate);

        if (UKismetMathLibrary::RandomBoolWithWeight(CriticalRate))
        {
            DreamEffectContext->SetDamageCritical(true);
            ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DMAttrStatics().CriticalDamageDef, EvaluationParameters, CriticalDamageAddition);
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

    // 等级差的伤害减免
    int32 LevelDiff = FMath::Max(0, TargetCharacter->GetCharacterLevel() - SourceCharacter->GetCharacterLevel());
    float LevelDiffDamageResist = UDMAbilitiesStatics::GetDamageResistFromLevelDiff(LevelDiff);

    // 防御力的伤害减免
    float DefenseDamageResist = UDMAbilitiesStatics::GetDamageResist(DefensePower);

    float DamageDone = AttackPower
        * (1 + PercentageDmgInc + IncreaseDamagePercentage + CriticalDamageAddition)
        * (1 + DamageTypeIncPercentage)
        * (1 - DamageFalloffPercentage - DefenseDamageResist - LevelDiffDamageResist);
    
    DamageDone = FMath::Max(DamageDone, 1.f);

    // 触发 GE.Executions.ConditionalGameplayEffects 
    OutExecutionOutput.MarkConditionalGameplayEffectsToTrigger();

    // 生命偷取计算
    float HealthStealPercentage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DMAttrStatics().HealthStealDef, EvaluationParameters, HealthStealPercentage);
    
    // 对源应用回血效果
    float TreatmentAmount = HealthStealPercentage * DamageDone;
    if (TreatmentAmount > 1.f)
    {
        FGameplayEffectSpec TreatmentEffectSpec(GetDefault<UDGE_DamageHealthSteal>(), SourceAbilitySystem->MakeEffectContext());
        TreatmentEffectSpec.SetSetByCallerMagnitude(CustomizeTags().SetByCaller_HealthSteal, TreatmentAmount);
        SourceAbilitySystem->ApplyGameplayEffectSpecToSelf(TreatmentEffectSpec);
    }

    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DMAttrStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
}

bool UDreamDamageCalculation::GetAttributeCaptureFromDamageType(EDDamageType DamageType, FGameplayEffectAttributeCaptureDefinition& OutDef) const
{
    bool bValid = true;
    
    switch (DamageType)
    {
    case EDDamageType::Weapon_Shotgun:

        OutDef = DMAttrStatics().WeaponDamageShotgunDef;
        break;

    case EDDamageType::Weapon_AssaultRifle:
            
        OutDef = DMAttrStatics().WeaponDamageAssaultRifleDef;
        break;

    case EDDamageType::Weapon_GrenadeLaunch:

        OutDef = DMAttrStatics().WeaponDamageGrenadeLaunchDef;
        break;
        
    case EDDamageType::Weapon_PrecisionRifle:

        OutDef = DMAttrStatics().WeaponDamagePrecisionRifleDef;
        break;
        
    case EDDamageType::Weapon_SniperRifle:

        OutDef = DMAttrStatics().WeaponDamageSniperRifleDef;
        break;
        
    default:
        bValid = false;
    }

    return bValid;
}
