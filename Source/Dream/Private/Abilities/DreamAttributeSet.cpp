// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/DreamAttributeSet.h"

#include "DGE_DamageHealthSteal.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Character/DCharacterPlayer.h"


UDreamAttributeSet::UDreamAttributeSet()
	: Health(1.f)
	, MaxHealth(1.f)
	, Shield(1.f)
	, MaxShield(1.f)
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
	DOREPLIFETIME(UDreamAttributeSet, Shield);
	DOREPLIFETIME(UDreamAttributeSet, MaxShield);
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

void UDreamAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, Shield, OldValue);
}

void UDreamAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDreamAttributeSet, MaxShield, OldValue);
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

void UDreamAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
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
	else if (Attribute == GetMaxShieldAttribute())
	{
		AdjustAttributeForMaxChange(Shield, MaxShield, NewValue, GetShieldAttribute());
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
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
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();

	// Compute the delta between old and new, if it is available
	/*float DeltaValue = 0;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		// If this was additive, store the raw delta value to be passed along later
		DeltaValue = Data.EvaluatedData.Magnitude;
	}*/

	// Get the Target actor, which should be our owner
	ADCharacterBase* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->OwnerActor.IsValid())
	{
		AActor* TargetActor = Data.Target.AbilityActorInfo->OwnerActor.Get();
		TargetCharacter = Cast<ADCharacterBase>(TargetActor);
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Get the Source actor
		/*ADCharacterBase* SourceCharacter = nullptr;
		if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->OwnerActor.IsValid())
		{
			SourceCharacter = Cast<ADCharacterBase>(Source->AbilityActorInfo->OwnerActor.Get());
		}*/

		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);

		if (LocalDamageDone > 0)
		{
			float CalcDamage = LocalDamageDone;

			const float OldShield = GetShield();
			if (OldShield > 0)
			{
				if (OldShield >= CalcDamage)
				{
					SetShield(OldShield - CalcDamage);
					CalcDamage = 0;
				}
				else
				{
					SetShield(0);
					CalcDamage -= OldShield; 
				}
			}

			if (CalcDamage > 0)
			{
				// Apply the health change and then clamp it
				const float OldHealth = GetHealth();
				SetHealth(FMath::Clamp(OldHealth - CalcDamage, 0.0f, GetMaxHealth()));
			}
			
			if (TargetCharacter)
			{
				TargetCharacter->HandleDamage(LocalDamageDone, Data.EffectSpec.GetEffectContext());
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle other health changes such as from healing or direct modifiers
		// First clamp it
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		
		if (TargetCharacter)
		{
			// Call for all health changes
			//TargetCharacter->HandleHealthChanged(DeltaValue, SourceTags);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthStealAttribute())
	{
		if (GetHealth() == 0.f)
		{
			return;
		}
		
		// 生命回复效果
		float HealthStealValue = GetHealthSteal();

		FGameplayEffectSpec TreatmentEffectSpec(GetDefault<UDGE_DamageHealthSteal>(), Context);
		TreatmentEffectSpec.SetSetByCallerMagnitude(UDGE_DamageHealthSteal::HSHealthSetByCallerTag, HealthStealValue);

		float SourceHealthValue = Source->GetNumericAttribute(DreamAttrStatics().HealthProperty);
		float SourceMaxHealthValue = Source->GetNumericAttribute(DreamAttrStatics().MaxHealthProperty);
		float OverflowCureAmount = FMath::Max(0.f, HealthStealValue + SourceHealthValue - SourceMaxHealthValue);
		
		if (OverflowCureAmount > 0.f)
		{
			TreatmentEffectSpec.SetSetByCallerMagnitude(UDGE_DamageHealthSteal::HSShieldSetByCallerTag, OverflowCureAmount);
		}

		Source->ApplyGameplayEffectSpecToSelf(TreatmentEffectSpec);
		
		SetHealthSteal(0.f);
	}
}
