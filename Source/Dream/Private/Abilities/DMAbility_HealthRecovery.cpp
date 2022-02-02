// Fill out your copyright notice in the Description page of Project Settings.


#include "DMAbility_HealthRecovery.h"

UDMAbility_HealthRecovery::UDMAbility_HealthRecovery()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}
