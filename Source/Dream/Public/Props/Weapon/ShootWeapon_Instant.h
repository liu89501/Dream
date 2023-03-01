#pragma once

#include "ShootWeapon.h"
#include "ShootWeapon_Instant.generated.h"

UCLASS()
class DREAM_API AShootWeapon_Instant : public AShootWeapon
{
	GENERATED_BODY()

public:

	AShootWeapon_Instant();
	

	UPROPERTY(EditAnywhere, Category = "Weapon|VFX")
	FWeaponTrailVFX TrailVfx;

	UPROPERTY(EditAnywhere, Category = "Weapon|VFX")
	FVector DecalSize;
	
public:

	virtual void BeginPlay() override;
	
protected:

	virtual void HandleSpawnProjectile(const FHitResult& HitResult) override;

	virtual void GetActualProjectileHitResult(const FHitResult& ViewHitResult, FHitResult& ActualHitResult) const override;
};