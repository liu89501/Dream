// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CharacterPreviewAnimInstance.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UCharacterPreviewAnimInstance : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API ICharacterPreviewAnimInstance
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponAnimID(int32 NewAnimID);

};
