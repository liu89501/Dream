// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LDedicatedServerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, meta = (CannotImplementInterfaceInBlueprint))
class ULDedicatedServerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API ILDedicatedServerInterface
{
	GENERATED_BODY()

	DECLARE_DELEGATE_OneParam(FLaunchComplete, const FString& /* ServerJoinAddress */)

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void LaunchDedicatedServer(const FString& LaunchParams) {};

	FLaunchComplete& GetCompleteDelegate()
	{
		return OnLaunchComplete;
	}

protected:

	FLaunchComplete OnLaunchComplete;

};
