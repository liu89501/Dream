#pragma once

#include "ShootWeapon.h"
#include "ShootWeapon_Instant.generated.h"

UCLASS()
class DREAM_API AShootWeapon_Instant : public AShootWeapon
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Weapon|VFX")
	FWeaponTrailVFX TrailVfx;
	
public:

	virtual void BeginPlay() override;
	
protected:

	virtual void HandleSpawnAmmo(const FHitResult& HitResult) override;
};