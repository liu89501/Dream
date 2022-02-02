// Fill out your copyright notice in the Description page of Project Settings.


#include "DMAttributeSet.h"

#include "DGE_DamageHealthSteal.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Character/DCharacterPlayer.h"


UDMAttributeSet::UDMAttributeSet()
	: Health(1.f)
	, MaxHealth(1.f)
	, Recovery(1.f)
	, AttackPower(1.0f)
	, DefensePower(1.0f)
	, Damage(0.0f)
	, HealthSteal(0.f)
{
}

void UDMAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDMAttributeSet, Health);
	DOREPLIFETIME(UDMAttributeSet, MaxHealth);
	DOREPLIFETIME(UDMAttributeSet, Recovery);
	DOREPLIFETIME(UDMAttributeSet, AttackPower);
	DOREPLIFETIME(UDMAttributeSet, DefensePower);
	DOREPLIFETIME(UDMAttributeSet, CriticalRate);
	DOREPLIFETIME(UDMAttributeSet, CriticalDamage);
}

void UDMAttributeSet::IncrementAttributes(const FEquipmentAttributes& Attributes)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();

	ASC->ApplyModToAttribute(DMAttrStatics().AttackPowerProperty, EGameplayModOp::Additive, Attributes.AttackPower);
	ASC->ApplyModToAttribute(DMAttrStatics().CriticalDamageProperty, EGameplayModOp::Additive, Attributes.CriticalDamage);
	ASC->ApplyModToAttribute(DMAttrStatics().CriticalRateProperty, EGameplayModOp::Additive, Attributes.CriticalRate);
	ASC->ApplyModToAttribute(DMAttrStatics().DefensePowerProperty, EGameplayModOp::Additive, Attributes.Defense);
	ASC->ApplyModToAttribute(DMAttrStatics().MaxHealthProperty, EGameplayModOp::Additive, Attributes.MaxHealth);
	ASC->ApplyModToAttribute(DMAttrStatics().HealthStealProperty, EGameplayModOp::Additive, Attributes.HealthSteal);
}

void UDMAttributeSet::UpdateAttributesBase(const FEquipmentAttributes& Attributes)
{
	SetAttackPower(Attributes.AttackPower);
	SetCriticalDamage(Attributes.CriticalDamage);
	SetCriticalRate(Attributes.CriticalRate);
	SetDefensePower(Attributes.Defense);
	SetMaxHealth(Attributes.MaxHealth);
	SetHealthSteal(Attributes.HealthSteal);
}

void UDMAttributeSet::InitAttributes(const FEquipmentAttributes& Attributes)
{
	InitAttackPower(Attributes.AttackPower);
	InitCriticalDamage(Attributes.CriticalDamage);
	InitCriticalRate(Attributes.CriticalRate);
	InitDefensePower(Attributes.Defense);
	InitMaxHealth(Attributes.MaxHealth);
	InitHealth(Attributes.MaxHealth);
	InitHealthSteal(Attributes.HealthSteal);
}

void UDMAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDMAttributeSet, Health, OldValue);
}

void UDMAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDMAttributeSet, MaxHealth, OldValue);
}

void UDMAttributeSet::OnRep_Recovery(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDMAttributeSet, Recovery, OldValue);
}

void UDMAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDMAttributeSet, AttackPower, OldValue);
}

void UDMAttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDMAttributeSet, DefensePower, OldValue);
}

void UDMAttributeSet::OnRep_CriticalRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDMAttributeSet, CriticalRate, OldValue);
}

void UDMAttributeSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDMAttributeSet, CriticalDamage, OldValue);
}

void UDMAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttribute(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UDMAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// This is called whenever attributes change, so for max health/mana we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxHealth.GetBaseValue());
	}
}

bool UDMAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	Super::PreGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		return GetHealth() > 0;
	}

	return true;
}

void UDMAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();

	ADCharacterBase* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->OwnerActor.IsValid())
	{
		AActor* TargetActor = Data.Target.AbilityActorInfo->OwnerActor.Get();
		TargetCharacter = Cast<ADCharacterBase>(TargetActor);
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = GetDamage();
		
		SetDamage(0.f);

		if (LocalDamageDone > 0)
		{
			const float OldHealth = GetHealth();
			SetHealth(FMath::Clamp(OldHealth - LocalDamageDone, 0.0f, GetMaxHealth()));
			
			if (TargetCharacter)
			{
				TargetCharacter->HandleDamage(LocalDamageDone, Data.EffectSpec.GetEffectContext());
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		//SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetHealthStealAttribute())
	{
		if (GetHealth() > 0.f)
		{
			float CurrentHealth = GetHealth();

			// 生命回复效果
			float HealthStealValue = GetHealthSteal();

			SetHealth(CurrentHealth + HealthStealValue);

			SetHealthSteal(0.f);
		}
	}
}
