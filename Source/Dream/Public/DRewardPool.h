// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DRewardPool.generated.h"

class UItemData;

UENUM()
enum class EProbabilityStrategy : uint8
{
	OneOf UMETA(ToolTip="从列表中按几率取得一个"),
    All UMETA(ToolTip="列表中的每一项奖励的获取几率都单独计算")
};

USTRUCT(BlueprintType)
struct FRewardItem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category=Reward)
	float Probability;

	UPROPERTY(EditAnywhere, Instanced, Category=Reward)
	UItemData* Reward;
};

USTRUCT(BlueprintType)
struct FRewardGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Reward)
	EProbabilityStrategy Strategy;

	UPROPERTY(EditAnywhere, Category=Reward)
	TArray<FRewardItem> RewardItems;
};

/**
 * 
 */
UCLASS(BlueprintType)
class DREAM_API UDRewardPool : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FRewardGroup> RewardGroup;

	UFUNCTION(BlueprintCallable)
	UItemData* GenerateRewards();
};
