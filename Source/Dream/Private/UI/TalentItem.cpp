// Fill out your copyright notice in the Description page of Project Settings.


#include "TalentItem.h"

#include "TalentGrid.h"

void UTalentItem::InitialTalentItem(const FTalentInfo& Info)
{
	TalentInfo = Info;
	InitialTalentWidget();
}

void UTalentItem::NativeRefreshTalentItem(bool bInLearnable)
{
	bLearnable = bInLearnable;
	RefreshTalentItem();
}

void UTalentItem::LearningTalent()
{
	TalentInfo.bLearned = true;
	bLearnable = false;
	if (UTalentGrid* TalentGrid = Cast<UTalentGrid>(GetParent()))
	{
		TalentGrid->GetLearnedCompleted().Broadcast(this);
	}
}

bool UTalentItem::IsLearnedTalent() const
{
	return TalentInfo.bLearned;
}
