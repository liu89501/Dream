// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TalentGrid.h"
#include "DreamGameplayAbility.h"
#include "DreamType.h"
#include "PlayerDataInterfaceType.h"
#include "TalentItem.h"

void UTalentGrid::InitialTalentGrid(const TArray<FTalentInfo>& Talents)
{
	for (UTalentItem* TalentItem : TalentWidgets)
	{
		if (Talents.IsValidIndex(TalentItem->TalentIndex))
		{
			TalentItem->InitialTalentItem(Talents[TalentItem->TalentIndex]);
		}
	}

	RefreshTalents();
}

void UTalentGrid::RefreshTalents()
{
	TMap<int32, bool> TalentGroup;

	for (UTalentItem* TalentItem : TalentWidgets)
	{
		bool& bGroupTalentLearned = TalentGroup.FindOrAdd(TalentItem->TalentInfo.TalentGroupId);
		bGroupTalentLearned |= TalentItem->TalentInfo.bLearned;
	}

	for (UTalentItem* TalentItem : TalentWidgets)
	{
		if (!TalentItem->TalentInfo.TalentId)
		{
			continue;
		}
			
		int32 TalentGroupId = TalentItem->TalentInfo.TalentGroupId;

		bool bLearnableTalent = true;
		
		if (TalentGroupId > 0)
		{
			int32 PrevGroupId = TalentGroupId - 1;
			bool& bLearnedPrevGroupTalent = TalentGroup.FindChecked(PrevGroupId);
			bool& bLearnedGroupTalent = TalentGroup.FindChecked(TalentGroupId);
			bLearnableTalent = bLearnedPrevGroupTalent & !bLearnedGroupTalent;
		}
			
		TalentItem->NativeRefreshTalentItem(bLearnableTalent);
	}
}

void UTalentGrid::GetLearnedTalents(TArray<int32>& TalentIdArray, TArray<TSubclassOf<class UDreamGameplayAbility>>& TalentClasses) const
{
	for (UTalentItem* TalentItem : TalentWidgets)
	{
		if (TalentItem->TalentInfo.bLearned)
		{
			TalentIdArray.Add(TalentItem->TalentInfo.TalentId);
			TalentClasses.Add(TalentItem->TalentInfo.TalentClass.TryLoadClass<UDreamGameplayAbility>());
		}
	}
}

void UTalentGrid::ResetAllTalents()
{
	for (UTalentItem* TalentItem : TalentWidgets)
	{
		TalentItem->TalentInfo.bLearned = false;
	}
}

void UTalentGrid::OnWidgetRebuilt()
{
	if (!IsDesignTime())
	{
		DREAM_NLOG(Error, TEXT("UTalentGrid::OnWidgetRebuilt"));
		
		for (UWidget* Widget : GetAllChildren())
		{
			if (UTalentItem* TalentItem = Cast<UTalentItem>(Widget))
			{
				TalentWidgets.Add(TalentItem);
			}
		}
	}
}
