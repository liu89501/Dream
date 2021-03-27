// Fill out your copyright notice in the Description page of Project Settings.

#include "Props/Weapon/ShootWeapon_Projectile.h"
#include "DCharacterPlayer.h"
#include "GameFramework/ProjectileMovementComponent.h"

void AShootWeapon_Projectile::BeginPlay()
{
	Super::BeginPlay();
}

void AShootWeapon_Projectile::HandleSpawnAmmo(const FHitResult& HitResult)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Projectile)
		{
			// 子弹的方向
			FRotator ProjectileDir = (HitResult.ImpactPoint - HitResult.TraceStart).Rotation();

			FActorSpawnParameters SpawnParam;
			SpawnParam.Owner = this;
			SpawnParam.Instigator = GetOwningShooter();

			ADProjectile* SpawnProjectile = GetWorld()->SpawnActor<ADProjectile>(Projectile, HitResult.TraceStart, ProjectileDir, SpawnParam);

			if (HitResult.bBlockingHit)
			{
				ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(GetOwner(), HitResult.GetActor());
				if (Attitude == ETeamAttitude::Hostile)
				{
					SpawnProjectile->Projectile->HomingTargetComponent = HitResult.GetComponent();
				}
			}
		}
	}
}
