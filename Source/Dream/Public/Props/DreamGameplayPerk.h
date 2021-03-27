// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "GameplayEffect.h"
#include "DreamGameplayPerk.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDreamGameplayPerk : public UObject
{
	GENERATED_BODY()

    UDreamGameplayPerk();

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Perk")
    TSubclassOf<UGameplayEffect> PerkEffect;

    /**
     * 触发的时间节点
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Perk")
    ETimeFrame TimeFrame;

public:

    UFUNCTION(BlueprintNativeEvent)
    FActiveGameplayEffectHandle ActivationPerkEffect(UAbilitySystemComponent* TargetAbilitySystem);
};
