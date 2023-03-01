// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMProjectile_Explosive.h"
#include "DMProjectile_Explosive_Player.generated.h"


UCLASS(Abstract)
class DREAM_API ADMProjectile_Explosive_Player : public ADMProjectile_Explosive
{
	GENERATED_BODY()

public:

	ADMProjectile_Explosive_Player();
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	TSubclassOf<UMatineeCameraShake> ExplodedCameraShake;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float InnerCameraShakeRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float OuterCameraShakeRadius;

public:

	virtual void OnRep_Impact() override;
};
