// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMCharacterType.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	None,
	Grounded,
	InAir,
	Mantling,
	RagDoll
};

UENUM(BlueprintType)
enum class EMovementAction : uint8
{
	None,
	LowMantle,
    HighMantle,
    Rolling
};

UENUM(BlueprintType)
enum class EOverlayState : uint8
{
	Default,
	Injured,
	HandsTied,
	Rifle,
	Pistol1H,
	Pistol2H,
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel
};

UENUM(BlueprintType)
enum class EMovementGait : uint8
{
	Walking,
	Running,
	Sprinting
};

UENUM(BlueprintType)
enum class EMantleType : uint8
{
	None,
	HighMantle,
	LowMantle,
	FallingCatch
};


UENUM(BlueprintType)
enum class EGroundedEntryState : uint8
{
	None,
	Roll
};

UENUM(BlueprintType)
enum class ERollingDirection : uint8
{
	F,
	FL,
	FR,
	B,
	BL,
	BR,
	R,
	L
};


