// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProjectileDamageInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UProjectileDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAM_API IProjectileDamageInterface
{
	GENERATED_BODY()

public:

	virtual void ApplyDamageEffect(const FHitResult& HitResult, const FVector& Origin) = 0;
};
