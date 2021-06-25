// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "UObject/Interface.h"
#include "PropsInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UPropsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IPropsInterface
{
	GENERATED_BODY()

public:

	virtual const FPropsInfo& GetPropsInfo() const = 0;

	virtual const FEquipmentAttributes& GetEquipmentAttributes() const = 0;
};
