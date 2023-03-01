// Fill out your copyright notice in the Description page of Project Settings.

#include "GearInterface.h"

FTransform IGearInterface::GetPreviewRelativeTransform() const
{
	return FTransform::Identity;
}

UDMUpgradeGearInfluence* IGearInterface::GetUpgradeAttributesInfluence() const
{
	return nullptr;
}
