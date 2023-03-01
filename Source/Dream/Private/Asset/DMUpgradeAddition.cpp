// Fill out your copyright notice in the Description page of Project Settings.

#include "DMUpgradeAddition.h"

float UDMUpgradeAddition::GetAdditionStrength(EPropsQuality Quality, uint8 Level) const
{
	switch (Quality)
	{
	case EPropsQuality::Normal:		return Normal->GetFloatValue(Level);
	case EPropsQuality::Advanced:	return Advanced->GetFloatValue(Level);
	case EPropsQuality::Rare:		return Rare->GetFloatValue(Level);
	case EPropsQuality::Epic:		return Epic->GetFloatValue(Level);
	case EPropsQuality::Legendary:	return Legendary->GetFloatValue(Level);
	}

	return 0.f;
}
