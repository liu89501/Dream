// Fill out your copyright notice in the Description page of Project Settings.

#include "DMShooterWeapon_Projectile.h"
#include "DEnemyShooter.h"
#include "DGameplayStatics.h"

UDMShooterWeapon_Projectile::UDMShooterWeapon_Projectile()
	: HorizontalConeHalfAngleRad(0.04f)
    , VerticalConeHalfAngleRad(0.02f)
{
}

void UDMShooterWeapon_Projectile::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		OwnerAI->OnApplyProjectileDamage.AddUObject(this, &UDMShooterWeapon_Projectile::OnApplyDamageEffect);
	}
}

void UDMShooterWeapon_Projectile::OpenFire(AActor* Hostile)
{
	if (ProjectileClass && Hostile)
	{
		SpawnProjectile(Hostile);
		MulticastAnimAndFX();
	}
}

void UDMShooterWeapon_Projectile::OnApplyDamageEffect(const FHitResult& HitResult, const FVector& Origin)
{
	checkf(OwnerAI, TEXT("OwnerAI Invalid"));

	UDGameplayStatics::ApplyGameplayEffect(OwnerAI->AbilitySystem, HitResult.GetActor(), DamageEffectClass, 1, false, DamageType);
}

ADProjectile* UDMShooterWeapon_Projectile::SpawnProjectile(AActor* Hostile)
{
	FTransform Transform;
	GetProjectileInitialTransform(Hostile, Transform);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = OwnerAI;
	SpawnParameters.Owner = OwnerAI;
	return GetWorld()->SpawnActor<ADProjectile>(ProjectileClass, Transform, SpawnParameters);
}

void UDMShooterWeapon_Projectile::GetProjectileInitialTransform(AActor* Hostile, FTransform& Transform)
{
	FVector Location = OwnerAI->GetPawnViewLocation();

	FRandomStream RandomStream(FMath::Rand());
	
	Transform.SetRotation(RandomStream.VRandCone(Hostile->GetActorLocation() - Location,
                                         HorizontalConeHalfAngleRad, VerticalConeHalfAngleRad).Rotation().Quaternion());

	Transform.SetTranslation(Location);
}

void UDMShooterWeapon_Projectile::MulticastAnimAndFX_Implementation()
{
	PlayAnimAndFX();
}
