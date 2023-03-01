// Fill out your copyright notice in the Description page of Project Settings.


#include "DMAbility_Shooting.h"
#include "DGameplayStatics.h"
#include "DGameplayTags.h"
#include "DMEffect_ShootingDamage.h"

UDMAbility_Shooting::UDMAbility_Shooting()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = CustomizeTags().Event_Firing;
	AbilityTriggers.Add(TriggerData);
}

void UDMAbility_Shooting::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (TriggerEventData)
	{
		UDGameplayStatics::ApplyGameplayEffectToAllActors(this, *TriggerEventData, UDMEffect_ShootingDamage::StaticClass(), false, EDDamageType::Weapon);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
}
