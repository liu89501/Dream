// Fill out your copyright notice in the Description page of Project Settings.


#include "DMEffect_HealthRecovery.h"
#include "DMAttributeSet.h"

UDMEffect_HealthRecovery::UDMEffect_HealthRecovery()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	Period = 0.5f;

	
}
