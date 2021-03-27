// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DreamType.h"
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

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, Category="Interface|Props")
	FPropsInfo GetPropsInfo() const;
};
