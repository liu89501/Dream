// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShootWeapon_Rifle.h"
#include "ShootWeapon_Sniper.generated.h"

/**
 *
 */
UCLASS()
class DREAM_API AShootWeapon_Sniper : public AShootWeapon_Rifle
{
	GENERATED_BODY()

public:

	AShootWeapon_Sniper();

	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* ScopeViewPoint;

	/* 瞄准时的枪口偏移 */
	UPROPERTY(EditAnywhere, Category = SniperWeapon)
	FVector AimMuzzleOffset;

	void SetWeaponAim(bool NewAimed) override;

protected:

	virtual void PostInitializeComponents() override;

	virtual void SetWeaponEnable(bool bEnable) override;

	virtual void OnAimEnded() override;

	virtual void GetMuzzlePoint(FVector& Point, FRotator& Direction) const override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnAimEnded"))
	void BP_OnAimChange();

private:

	FTransform LastCameraTransform;
};
