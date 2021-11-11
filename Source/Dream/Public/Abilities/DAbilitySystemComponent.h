// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	void ClearAbilityFromClass(UClass* AbilityClass);
};
