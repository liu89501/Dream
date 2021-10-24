// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/DreamAttributeSet.h"

#include "DGE_DamageHealthSteal.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Character/DCharacterPlayer.h"


UDreamAttributeSet::UDreamAttributeSet()
	: Health(1.f)
	, MaxHealth(1.f)
	, AttackPower(1.0f)
	, DefensePower(1.0f)
	, Damage(0.0f)
	, HealthSteal(0.f)
{
}

void UDreamAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDreamAttributeSet, Health);
	DOREPLIFETIME(UDreamAttributeSet, MaxHealth);
	DOREPLIFETIME(UDreamAttributeSet, AttackPower);
	DOREPLIFETIME(UDreamAttributeSet, DefensePower);
	DOREPLIFETIME(UDreamAttributeSet, CriticalRate);
	DOREPLIFETIME(UDreamAttributeSet, CriticalDamage);
}

void UDreamAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, Health, OldValue);
}

void UDreamAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, MaxHealth, OldValue);
}

void UDreamAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, AttackPower, OldValue);
}

void UDreamAttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, DefensePower, OldValue);
}

void UDreamAttributeSet::OnRep_CriticalRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, CriticalRate, OldValue);
}

void UDreamAttributeSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, CriticalDamage, OldValue);
}

void UDreamAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
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

void UDreamAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
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

bool UDreamAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	Super::PreGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		return GetHealth() > 0;
	}

	return true;
}

void UDreamAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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
