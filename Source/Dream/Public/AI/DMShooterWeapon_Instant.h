// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShootWeapon.h"
#include "DMShooterWeaponComponent.h"
#include "DMShooterWeapon_Instant.generated.h"


UCLASS(meta=(BlueprintSpawnableComponent, DisplayName="AIWeapon_Instant"))
class DREAM_API UDMShooterWeapon_Instant : public UDMShooterWeaponComponent
{
	GENERATED_BODY()

	UDMShooterWeapon_Instant();

public:	

	UPROPERTY(EditAnywhere, Category = AISettings)
	FWeaponTrailVFX TrailVfx;

public:

	virtual void OpenFire(AActor* Hostile) override;

protected:

	UFUNCTION(Unreliable, NetMulticast)
    void MulticastFiringFX(const FVector_NetQuantize10& ImpactPoint);

	void AttemptApplyDamage(FVector TraceEnd);
};
