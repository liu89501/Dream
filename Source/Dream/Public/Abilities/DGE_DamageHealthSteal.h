// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DGE_DamageHealthSteal.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDGE_DamageHealthSteal : public UGameplayEffect
{
	GENERATED_BODY()

	UDGE_DamageHealthSteal();

public:

	static FGameplayTag HealthStealSetByCallerTag;
};
