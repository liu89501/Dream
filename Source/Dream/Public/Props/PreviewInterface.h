// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PreviewInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UPreviewInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IPreviewInterface
{
	GENERATED_BODY()

public:

	virtual FTransform GetPreviewRelativeTransform() const;
};
