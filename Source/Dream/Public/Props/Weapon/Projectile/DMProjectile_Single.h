// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DProjectile.h"
#include "DMProjectile_Single.generated.h"


UCLASS(Abstract)
class DREAM_API ADMProjectile_Single : public ADProjectile
{
	GENERATED_BODY()

protected:

	virtual void OnRep_Impact() override;

	virtual void ApplyProjectileDamage(const FHitResult& HitResult) override;
};
