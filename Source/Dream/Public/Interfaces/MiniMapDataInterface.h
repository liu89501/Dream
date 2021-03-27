// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "MiniMapDataComponent.h"
#include "UObject/Interface.h"
#include "MiniMapDataInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, meta = (CannotImplementInterfaceInBlueprint))
class UMiniMapDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IMiniMapDataInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual UMiniMapDataComponent* GetMiniMapDataComponent() const = 0;
};
