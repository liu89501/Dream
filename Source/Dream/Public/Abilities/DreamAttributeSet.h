// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DreamAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class DREAM_API UDreamAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	
	UDreamAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** Current Health, when 0 we expect owner to die. Capped by MaxHealth */
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, Health)

	/** MaxHealth is its own attribute, since GameplayEffects may modify it */
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, MaxHealth)

	/** AttackPower of the attacker is multiplied by the base Damage to reduce health, so 1.0 means no bonus */
	UPROPERTY(BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, AttackPower)

	/** Base Damage is divided by DefensePower to get actual damage done, so 1.0 means no bonus */
	UPROPERTY(BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_DefensePower)
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, DefensePower)

	/** 暴击率 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_CriticalRate)
	FGameplayAttributeData CriticalRate;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, CriticalRate)
	
	/** 暴击伤害 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_CriticalDamage)
	FGameplayAttributeData CriticalDamage;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, CriticalDamage)

	/** Damage is a 'temporary' attribute used by the DamageExecution to calculate final damage, which then turns into -Health */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, Damage)

	/**
	 * 生命偷取
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData HealthSteal;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, HealthSteal)


	/* --------------------------------- 武器类型增伤 ------------------------------- */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData WeaponDamageAssaultRifle;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, WeaponDamageAssaultRifle)
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData WeaponDamageGrenadeLaunch;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, WeaponDamageGrenadeLaunch)
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData WeaponDamageShotgun;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, WeaponDamageShotgun)
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData WeaponDamageSniperRifle;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, WeaponDamageSniperRifle)
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData WeaponDamagePrecisionRifle;
	ATTRIBUTE_ACCESSORS(UDreamAttributeSet, WeaponDamagePrecisionRifle)

protected:
	/** Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) */
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const;
	
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_DefensePower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalRate(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);
	
};

#define D_DECLARE_ATTRIBUTE_CAPTUREDEF(P) \
FProperty* P##Property; \
FGameplayEffectAttributeCaptureDefinition P##Def; \

#define D_DEFINE_ATTRIBUTE_CAPTUREDEF(S, P, T, B) \
{ \
P##Property = FindFieldChecked<FProperty>(S::StaticClass(), GET_MEMBER_NAME_CHECKED(S, P)); \
P##Def = FGameplayEffectAttributeCaptureDefinition(P##Property, EGameplayEffectAttributeCaptureSource::T, B); \
}

struct DreamAttributeStatics
{
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalRate);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(MaxHealth);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(HealthSteal);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamageAssaultRifle);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamageGrenadeLaunch);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamageShotgun);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamageSniperRifle);
	D_DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamagePrecisionRifle);

	DreamAttributeStatics()
	{
		// Capture the Target's DefensePower attribute. Do not snapshot it, because we want to use the health value at the moment we apply the execution.
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, DefensePower, Target, false);

		// Capture the Source's AttackPower. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
		// (imagine we fire a projectile: we create the GE Spec when the projectile is fired. When it hits the target, we want to use the AttackPower at the moment
		// the projectile was launched, not when it hits).
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, AttackPower, Source, true);

		// Also capture the source's raw Damage, which is normally passed in directly via the execution
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, Damage, Source, true);
        
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, CriticalRate, Source, true);
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, CriticalDamage, Source, true);
		
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, MaxHealth, Source, false);
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, Health, Source, false);
		
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, HealthSteal, Source, true);
		
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, WeaponDamageAssaultRifle, Source, true);
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, WeaponDamageGrenadeLaunch, Source, true);
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, WeaponDamageShotgun, Source, true);
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, WeaponDamageSniperRifle, Source, true);
		D_DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, WeaponDamagePrecisionRifle, Source, true);
	}
};

static const DreamAttributeStatics& DreamAttrStatics()
{
	static DreamAttributeStatics AttrStatics;
	return AttrStatics;
}
