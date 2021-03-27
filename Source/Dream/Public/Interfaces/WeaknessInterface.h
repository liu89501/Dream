// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"
#include "WeaknessInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UWeaknessInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IWeaknessInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual float GetWeaknessIncreaseDamagePercentage(const FName& BoneName) = 0;
};
