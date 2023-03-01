// Fill out your copyright notice in the Description page of Project Settings.

#include "Props/Weapon/ShootWeapon_Projectile.h"
#include "DCharacterPlayer.h"


void AShootWeapon_Projectile::HandleSpawnProjectile(const FHitResult& HitResult)
{
	if (GetLocalRole() == ROLE_Authority && Projectile && OwningShooter)
	{
		const FVector& EndPoint = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
			
		FVector MuzzleLoc;
		FRotator MuzzleRot;
		
		// 这里不使用枪口插槽的位置， 因为在服务器上动画状态可能与客户端不一样(客户端瞄准，但服务器并不是)
		OwningShooter->GetActorEyesViewPoint(MuzzleLoc, MuzzleRot);

		FRotator ProjectileDir = (EndPoint - MuzzleLoc).Rotation();

		FActorSpawnParameters SpawnParam;
		SpawnParam.Owner = this;
		SpawnParam.Instigator = GetOwningShooter();

		GetWorld()->SpawnActor<ADProjectile>(Projectile, MuzzleLoc, ProjectileDir, SpawnParam);
		
	}
}
