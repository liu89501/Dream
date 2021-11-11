// Fill out your copyright notice in the Description page of Project Settings.


#include "DRewardPool.h"
#include "DreamType.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/KismetMathLibrary.h"

UItemData* UDRewardPool::GenerateRewards(ADPlayerState* PlayerState)
{
	if (PlayerState == nullptr)
	{
		return nullptr;
	}

	UItemDataContainer* DataContainer = NewObject<UItemDataContainer>();

	for (const FRewardGroup Group : RewardGroup)
	{
		if (Group.Strategy == EProbabilityStrategy::OneOf)
		{
			TArray<float> Probability;
			TArray<UItemData*> RewardsTemplate;
			for (const FRewardItem& RewardItem : Group.RewardItems)
			{
				check(RewardItem.Reward);

				RewardsTemplate.Add(RewardItem.Reward);
				Probability.Add(RewardItem.Probability);
			}

			int32 Index = FRandomProbability::RandomProbability(Probability, PROBABILITY_UINT);
			if (Index > INDEX_NONE)
			{
				UItemData* NewItemData = NewObject<UItemData>(PlayerState,
				                                              RewardsTemplate[Index]->GetClass(), NAME_None, RF_NoFlags,
				                                              RewardsTemplate[Index]);
				NewItemData->PlayerId = PlayerState->GetPlayerId();

				DataContainer->AddItem(NewItemData);
			}
		}
		else
		{
			for (const FRewardItem& RewardItem : Group.RewardItems)
			{
				check(RewardItem.Reward);

				if (UKismetMathLibrary::RandomBoolWithWeight(RewardItem.Probability / PROBABILITY_UINT))
				{
					UItemData* NewItemData = NewObject<UItemData>(PlayerState,
					                                              RewardItem.Reward->GetClass(), NAME_None, RF_NoFlags,
					                                              RewardItem.Reward);

					NewItemData->PlayerId = PlayerState->GetPlayerId();
					DataContainer->AddItem(NewItemData);
				}
			}
		}
	}

	return DataContainer;
}
