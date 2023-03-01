// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMShooterWeapon_Projectile.h"
#include "DMShooterWeapon_SuggestProjectile.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent, DisplayName="AIWeapon_SuggestProjectile"))
class DREAM_API UDMShooterWeapon_SuggestProjectile : public UDMShooterWeapon_Projectile
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=SuggestProjectile)
	bool bFavorHighArc;

public:

	virtual void OpenFire(AActor* Hostile) override;
};
