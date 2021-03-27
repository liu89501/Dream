#pragma once

#include "ShootWeapon.h"
#include "ShootWeapon_Projectile.generated.h"

UCLASS()
class DREAM_API AShootWeapon_Projectile : public AShootWeapon
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = WeaponProjectile)
	TSubclassOf<ADProjectile> Projectile;
	
public:

	virtual void BeginPlay() override;

protected:

	virtual void HandleSpawnAmmo(const FHitResult& HitResult) override;
};