// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "GameplayEffect.h"
#include "PerkEffectSystemComponent.h"

#include "DreamGameplayPerk.generated.h"

UENUM()
enum class EApplyTarget : uint8
{
	Source UMETA(DisplayName = "应用到自身", ToolTip = "GE 应用到自身"),
	Target UMETA(DisplayName = "应用到目标", ToolTip = "GE 应用到目标")
};

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDreamGameplayPerk : public UObject
{
	GENERATED_BODY()

    UDreamGameplayPerk();

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perk")
    TSubclassOf<UGameplayEffect> PerkEffect;

    /**
     * 触发的时间节点
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perk")
    EOpportunity Opportunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perk")
	EApplyTarget ApplyTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perk")
	bool bLocalRunning;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category="Perk")
	class UDGameplayEffectUIData* UIData;

public:

    UFUNCTION(BlueprintNativeEvent)
    FActiveGameplayEffectHandle ActivationPerkEffect(const FActivationOpportunityParams& OpportunityParams);
    
protected:

	UFUNCTION(BlueprintCallable, Category="Perk")
	AActor* SpawnActor(TSubclassOf<AActor> ActorClass, const FVector& Location,
		ESpawnActorCollisionHandlingMethod CollisionOverride, const FActivationOpportunityParams& OpportunityParams);
};
