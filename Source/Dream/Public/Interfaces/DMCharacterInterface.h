// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMCharacterType.h"
#include "UObject/Interface.h"
#include "DMCharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UDMCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IDMCharacterInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void GetDefaultValues(float& WalkSpeed, float& RunSpeed, float& SprintSpeed);

	UFUNCTION(BlueprintNativeEvent)
	void GetBasicValues(
		FRotator& ControlRotation,
		FVector& Acceleration,
		FRotator& AimingRotation,
		float& Speed,
		bool& bIsMoving,
		bool& bHasMovementInput
	);

	UFUNCTION(BlueprintNativeEvent)
	void GetCharacterStates(EMovementState& MovementState, EMovementGait& Gait);
};
