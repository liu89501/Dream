#pragma once

#include "ShootWeapon.h"
#include "ShootWeapon_Instant.generated.h"

UCLASS()
class DREAM_API AShootWeapon_Instant : public AShootWeapon
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = Weapon)
	class UNiagaraSystem* BulletTrace;

	UPROPERTY(EditAnywhere, Category = Weapon)
	FName TraceParamName;
	
public:

	virtual void BeginPlay() override;
	
protected:

	virtual void HandleSpawnAmmo(const FHitResult& HitResult) override;
};