// Fill out your copyright notice in the Description page of Project Settings.


#include "DRewardPool.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/KismetMathLibrary.h"

void HandleItemDetails(const FRewardItem& RewardItem, FItemListHandle& DropRewards, FItemListHandle& DirectRewards)
{
	if (RewardItem.Reward)
	{
		TSharedPtr<FItem> ItemStruct = RewardItem.Reward->MakeItemStruct();

		if (RewardItem.bDrop)
		{
			DropRewards.AddItem(ItemStruct);
		}
		else
		{
			DirectRewards.AddItem(ItemStruct);
		}
	}
}

void UDRewardPool::GenerateRewards(FItemListHandle& DropRewards, FItemListHandle& DirectRewards)
{
	for (const FRewardGroup Group : RewardGroup)
	{
		if (Group.Strategy == EProbabilityStrategy::OneOf)
		{
			TArray<float> Probability;
			Probability.SetNumUninitialized(Group.RewardItems.Num());
			for (int32 N = 0; N < Probability.Num(); N++)
			{
				Probability[N] = Group.RewardItems[N].Probability;
			}

			int32 Index = FRandomProbability::RandomProbability(Probability);
			if (Index > INDEX_NONE)
			{
				HandleItemDetails(Group.RewardItems[Index], DropRewards, DirectRewards);
			}
		}
		else
		{
			for (const FRewardItem& RewardItem : Group.RewardItems)
			{
				if (UKismetMathLibrary::RandomBoolWithWeight(RewardItem.Probability))
				{
					HandleItemDetails(RewardItem, DropRewards, DirectRewards);
				}
			}
		}
	}
}

