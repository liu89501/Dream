// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DreamGameplayAbility.h"
#include "Engine/DataAsset.h"
#include "DAbilityPerk.generated.h"

USTRUCT()
struct FPerkItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float Probability;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UDreamGameplayAbility> Ability;
};

/**
 * 
 */
UCLASS(EditInlineNew, Blueprintable)
class DREAM_API UDAbilityPerk : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere)
	int32 NumberOfExtractions;
	
	UPROPERTY(EditAnywhere)
	TArray<FPerkItem> Items;

	void Extraction();
};
