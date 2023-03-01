#pragma once

#include "ShootWeapon.h"
#include "DProjectile.h"
#include "ShootWeapon_Projectile.generated.h"

UCLASS()
class DREAM_API AShootWeapon_Projectile : public AShootWeapon
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = WeaponProjectile)
	TSubclassOf<ADProjectile> Projectile;

protected:

	virtual void HandleSpawnProjectile(const FHitResult& HitResult) override;
};