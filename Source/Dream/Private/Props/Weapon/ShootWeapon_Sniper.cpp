// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Weapon/ShootWeapon_Sniper.h"
#include "Camera/CameraComponent.h"
#include "Character/DCharacterPlayer.h"
#include "Character/DPlayerCameraManager.h"

AShootWeapon_Sniper::AShootWeapon_Sniper()
{
	/*ScopePoint = CreateDefaultSubobject<USceneComponent>(TEXT("ScopePoint"));
	ScopePoint->SetupAttachment(RootComponent);*/
}

void AShootWeapon_Sniper::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AShootWeapon_Sniper::SetWeaponEnable(bool bEnable)
{
	Super::SetWeaponEnable(bEnable);

	if (!bEnable)
	{
		OnAimEnded();
	}
}

void AShootWeapon_Sniper::OnAimEnded()
{
	BP_OnAimChange();
	if (bAimed)
	{
		GetOwningShooter()->GetRootComponent()->SetVisibility(false, true);
	}
	else
	{
		GetOwningShooter()->GetRootComponent()->SetVisibility(true, true);
	}
}

void AShootWeapon_Sniper::SetWeaponAim(bool NewAimed)
{
	Super::SetWeaponAim(NewAimed);
}

void AShootWeapon_Sniper::GetMuzzlePoint(FVector& Point, FRotator& Direction) const
{
	ADCharacterPlayer* Shooter = GetOwningShooter();
	if (Shooter->IsLocallyControlled() && bAimed)
	{
		Direction = Shooter->TPCamera->GetComponentRotation();
		Point = Shooter->TPCamera->GetComponentLocation() + Direction.RotateVector(AimMuzzleOffset);
	}
	else
	{
		Super::GetMuzzlePoint(Point, Direction);
	}
}

void AShootWeapon_Sniper::AimTimelineTick(float Value) const
{
	Super::AimTimelineTick(Value);
	GetOwningShooter()->TPCamera->PostProcessSettings.DepthOfFieldFocalDistance = FMath::Lerp(0.f, 20.f, Value);
}
