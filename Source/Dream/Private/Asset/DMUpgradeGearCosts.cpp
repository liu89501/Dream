// Fill out your copyright notice in the Description page of Project Settings.

#include "DMUpgradeGearCosts.h"

#define EVAL(Level, Base, V1, V2) FMath::CeilToInt(Base * FMath::Pow(V1, V2 * Level))

void UDMUpgradeGearCosts::GetUpgradeCost(int32 GearLevel, int32 ItemGuid, TArray<FAcquisitionCost>& UpgradeCosts)
{
	EPropsQuality PropsQuality = ItemUtils::GetItemQuality(ItemGuid);

	TArray<FCostItem>* CostsPtr = nullptr;
	
	switch (PropsQuality)
	{
	case EPropsQuality::Normal:		CostsPtr = &Normal;		break;
	case EPropsQuality::Advanced:	CostsPtr = &Advanced;	break;
	case EPropsQuality::Rare:		CostsPtr = &Rare;		break;
	case EPropsQuality::Epic:		CostsPtr = &Epic;		break;
	case EPropsQuality::Legendary:	CostsPtr = &Legendary;	break;
	}

	//TArray<FAcquisitionCost> UpgradeCosts;
	
	if (CostsPtr)
	{
		TArray<FCostItem>& CostItems = *CostsPtr;
		for (const FCostItem& Cost : CostItems)
		{
			FAcquisitionCost UpgradeCost;
			
			if (Cost.MinLevel > GearLevel)
			{
				continue;
			}

			UpgradeCost.CostAmount = EVAL(GearLevel, Cost.BaseNum, Cost.Var1, Cost.Var2);
			UpgradeCost.ItemGuid = Cost.ItemGuidHandle.ItemGuid;
			
			UpgradeCosts.Add(UpgradeCost);
		}
	}
}

void UDMUpgradeGearCosts::GetUpgradeCostHandle(int32 GearLevel, int32 ItemGuid, FCostsHandle& Handle)
{
	Handle.Costs = MakeShared<TArray<FAcquisitionCost>>();
	GetUpgradeCost(GearLevel, ItemGuid, *Handle.Costs);
}
