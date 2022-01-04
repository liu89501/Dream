// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TalentGrid.h"
#include "DreamGameplayAbility.h"
#include "DreamType.h"
#include "PlayerDataInterfaceType.h"
#include "TalentItem.h"

void UTalentGrid::InitialTalentGrid(const TArray<FTalentInfo>& Talents)
{
	//TArray<UWidget*> Children = GetAllChildren();
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
		bool& bGroupTalentLearned = TalentGroup.FindOrAdd(TalentItem->TalentGroup);
		bGroupTalentLearned |= TalentItem->TalentInfo.bLearned;
	}

	for (UTalentItem* TalentItem : TalentWidgets)
	{
		if (!TalentItem->TalentInfo.TalentId)
		{
			continue;
		}
			
		int32 TalentGroupId = TalentItem->TalentGroup;

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

void UTalentGrid::GetLearnedTalents(int64& LearnedTalents, TArray<TSubclassOf<class UDreamGameplayAbility>>& TalentClasses) const
{
	int64 Temp = 0;
	for (UTalentItem* TalentItem : TalentWidgets)
	{
		if (TalentItem->TalentInfo.bLearned)
		{
			Temp |= 1LL << TalentItem->TalentInfo.TalentIndex;
			TalentClasses.Add(TalentItem->TalentInfo.TalentClass);
		}
	}

	LearnedTalents = Temp;
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
		for (UWidget* Widget : GetAllChildren())
		{
			if (UTalentItem* TalentItem = Cast<UTalentItem>(Widget))
			{
				TalentWidgets.Add(TalentItem);
			}
		}
	}
}
