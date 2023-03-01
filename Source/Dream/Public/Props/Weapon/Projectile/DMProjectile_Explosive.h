// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DProjectile.h"
#include "DMProjectile_Explosive.generated.h"


UCLASS(Abstract)
class DREAM_API ADMProjectile_Explosive : public ADProjectile
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	class UParticleSystem* HitParticles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	class USoundCue* HitSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float DamageRadius;

public:

	virtual void OnRep_Impact() override;

	virtual void ApplyProjectileDamage(const FHitResult& HitResult) override;
};
