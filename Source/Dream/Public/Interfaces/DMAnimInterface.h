// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMCharacterType.h"
#include "UObject/Interface.h"
#include "DMAnimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UDMAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IDMAnimInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetOverlayOverrideState(int32 OverrideStateID);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetGroundedEntryState(EGroundedEntryState EntryState);

	/** 仅在预览角色时才会用到这个函数 */
	UFUNCTION(BlueprintImplementableEvent)
	void SetOverlayDetailState(int32 OverlayDetailState);
};
