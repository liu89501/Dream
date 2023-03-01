// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DProjectile.h"
#include "DMShooterWeaponComponent.h"
#include "DMShooterWeapon_Projectile.generated.h"


UCLASS(meta=(BlueprintSpawnableComponent, DisplayName="AIWeapon_Projectile"))
class DREAM_API UDMShooterWeapon_Projectile : public UDMShooterWeaponComponent
{
	GENERATED_BODY()

public:

	UDMShooterWeapon_Projectile();	

	UPROPERTY(EditDefaultsOnly, Category = AIWeaponSettings)
	TSubclassOf<ADProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = AIWeaponSettings)
	float HorizontalConeHalfAngleRad;

	UPROPERTY(EditDefaultsOnly, Category = AIWeaponSettings)
	float VerticalConeHalfAngleRad;
	
public:

	virtual void BeginPlay() override;

	virtual void OpenFire(AActor* Hostile) override;

	virtual void OnApplyDamageEffect(const FHitResult& HitResult, const FVector& Origin);


protected:

	class ADProjectile* SpawnProjectile(AActor* Hostile);

	void GetProjectileInitialTransform(AActor* Hostile, FTransform& Transform);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastAnimAndFX();
};

