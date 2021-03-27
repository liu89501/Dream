// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Weapon/ShootWeapon_Sniper.h"
#include "Camera/CameraComponent.h"
#include "Character/DCharacterPlayer.h"
#include "Components/ArrowComponent.h"

AShootWeapon_Sniper::AShootWeapon_Sniper()
{
	ScopeViewPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("ScopeViewPoint"));
	ScopeViewPoint->SetupAttachment(RootComponent);
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

	if (ADCharacterPlayer* Shooter = GetOwningShooter())
	{
		if (bAimed)
		{
			Shooter->TPCamera->SetWorldTransform(ScopeViewPoint->GetComponentTransform());
			WeaponMesh->SetVisibility(false, true);
		}
		else
		{
			WeaponMesh->SetVisibility(true, true);
			Shooter->TPCamera->SetRelativeTransform(LastCameraTransform);
			LastCameraTransform = Shooter->TPCamera->GetRelativeTransform();
		}
	}
}

void AShootWeapon_Sniper::SetWeaponAim(bool NewAimed)
{
	Super::SetWeaponAim(NewAimed);
}

void AShootWeapon_Sniper::GetMuzzlePoint(FVector& Point, FRotator& Direction) const
{
	if (bAimed)
	{
		ADCharacterPlayer* Shooter = GetOwningShooter();
		if (Shooter && Shooter->IsLocallyControlled())
		{
			Direction = Shooter->TPCamera->GetComponentRotation();
			Point = Shooter->TPCamera->GetComponentLocation() + Direction.RotateVector(AimMuzzleOffset);
			return;
		}
	}

	Super::GetMuzzlePoint(Point, Direction);
}

//void AShootWeapon_Sniper::AimTimelineTick(float Value)
//{
//	Super::AimTimelineTick(Value);
//	ADCharacterPlayer* Shooter = GetOwningShooter();
//
//	Shooter->TPCamera->PostProcessSettings.DepthOfFieldFstop = FMath::Lerp(0.f, 30.f, Value);
//	Shooter->TPCamera->PostProcessSettings.DepthOfFieldFocalDistance = FMath::Lerp(0.f, 20.f, Value);
//}
