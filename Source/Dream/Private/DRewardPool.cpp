// Fill out your copyright notice in the Description page of Project Settings.


#include "DRewardPool.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/KismetMathLibrary.h"

void UDRewardPool::GenerateRewards(TArray<UItemData*>& Rewards)
{
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

			int32 Index = FRandomProbability::RandomProbability(Probability);
			if (Index > INDEX_NONE)
			{
				Rewards.Add(HandleItemDetails(RewardsTemplate[Index]));
			}
		}
		else
		{
			for (const FRewardItem& RewardItem : Group.RewardItems)
			{
				check(RewardItem.Reward);

				if (UKismetMathLibrary::RandomBoolWithWeight(RewardItem.Probability))
				{
					Rewards.Add(HandleItemDetails(RewardItem.Reward));
				}
			}
		}
	}
}

UItemData* UDRewardPool::HandleItemDetails(UItemData* SelectedTemplate) const
{
	if (SelectedTemplate->IsA<UItemDataEquipment_Random>())
	{
		UItemDataEquipment_Random* EquipmentRandom = Cast<UItemDataEquipment_Random>(SelectedTemplate);
		return EquipmentRandom->CastToEquipment();
	}

	return NewObject<UItemData>(GetTransientPackage(),
                         SelectedTemplate->GetClass(),
                         NAME_None, RF_NoFlags,
                         SelectedTemplate);
}
