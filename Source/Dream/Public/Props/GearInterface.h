// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DMUpgradeGearInfluence.h"
#include "UObject/Interface.h"
#include "GearInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UGearInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IGearInterface
{
	GENERATED_BODY()

public:

	virtual FTransform GetPreviewRelativeTransform() const;

	virtual UDMUpgradeGearInfluence* GetUpgradeAttributesInfluence() const;
};
