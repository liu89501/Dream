// Fill out your copyright notice in the Description page of Project Settings.

#include "DMUpgradeGearInfluence.h"

void UDMUpgradeGearInfluence::AttemptCalculateAddition(FEquipmentAttributes& Attributes, float UpgradeAdditionMagnitude) const
{
	for (const FAttributeHandle& Handle : InfluenceProperties)
	{
		FFloatProperty* AttributeProperty = Handle.AttributeProperty.Get();

		if (AttributeProperty)
		{
			if (float* AttributeValue = AttributeProperty->GetPropertyValuePtr_InContainer(&Attributes))
			{
				*AttributeValue *= 1 + UpgradeAdditionMagnitude;
			}
		}
	}
}

bool UDMUpgradeGearInfluence::IsInfluence(const FAttributeHandle& Attribute) const
{
	return InfluenceProperties.Find(Attribute) != INDEX_NONE;
}
