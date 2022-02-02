// Fill out your copyright notice in the Description page of Project Settings.

#include "Props/Weapon/ShootWeapon_Instant.h"
#include "DGameplayStatics.h"
#include "DMProjectSettings.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void AShootWeapon_Instant::BeginPlay()
{
	Super::BeginPlay();
}

void AShootWeapon_Instant::HandleSpawnAmmo(const FHitResult& HitResult)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		FVector MuzzleLocation;
		FRotator MuzzleRotation;
		GetMuzzlePoint(MuzzleLocation, MuzzleRotation);
		UDGameplayStatics::SpawnWeaponTrailParticles(this, TrailVfx, MuzzleLocation, HitResult.ImpactPoint);
	}

	if (HitResult.bBlockingHit)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			ApplyPointDamage(HitResult);
		}
		else
		{
			EPhysicalSurface PhysicalSurface = HitResult.PhysMaterial.IsValid() ? 
                HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType_Default;

			const FSurfaceImpactEffect& SurfaceEffect = GSProject->GetSurfaceImpactEffect(PhysicalSurface);

			FRotator OrientationRotator = HitResult.ImpactNormal.ToOrientationRotator();
			OrientationRotator.Pitch -= 90.f;

			UGameplayStatics::SpawnEmitterAtLocation(this, SurfaceEffect.ImpactParticles, HitResult.ImpactPoint, 
                                                     OrientationRotator, FVector::OneVector, true, EPSCPoolMethod::AutoRelease);
					
			UGameplayStatics::SpawnSoundAtLocation(this, SurfaceEffect.ImpactSound, HitResult.ImpactPoint);
		}
	}
}
