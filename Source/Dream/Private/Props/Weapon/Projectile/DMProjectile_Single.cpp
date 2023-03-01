// Fill out your copyright notice in the Description page of Project Settings.


#include "DMProjectile_Single.h"
#include "DMProjectSettings.h"
#include "DProjectile.h"
#include "DreamType.h"
#include "PhysicalMaterial.h"
#include "ProjectileDamageInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

void ADMProjectile_Single::OnRep_Impact()
{
	FVector Location = GetActorLocation();

	FCollisionQueryParams CollisionQuery;
	CollisionQuery.bReturnPhysicalMaterial = true;
	CollisionQuery.AddIgnoredActor(this);
	CollisionQuery.AddIgnoredActor(GetInstigator());

	FVector TraceEnd = Location + GetActorRotation().Vector().GetSafeNormal() * 100.f;

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult,
													Location, TraceEnd,
													ECC_Visibility,
													CollisionQuery);

#if DM_PROJECTILE_DEBUG

	int32 DebugValue = DMProjectileCVar::CVarDebugProjectileTrace.GetValueOnAnyThread();
	if (DebugValue == 1)
	{
		DrawDebugLine(GetWorld(), Location, TraceEnd, FColor::Red, false, 5.f, 0, 1.f);

		if (bHit)
		{
			DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Green, false, 5.f, 0);
		}
	}

#endif

	EPhysicalSurface SurfaceType = bHit ? HitResult.PhysMaterial->SurfaceType : EPhysicalSurface::SurfaceType_Default;

	const FSurfaceImpactEffect& SurfaceEffect = GSProject->GetSurfaceImpactEffect(SurfaceType);

	FRotator Rotation = GetActorRotation();
	if (SurfaceEffect.ImpactParticles)
	{
		FRotator ParticleRot(Rotation.Pitch - 90.f, Rotation.Yaw, Rotation.Roll);
		UGameplayStatics::SpawnEmitterAtLocation(this, SurfaceEffect.ImpactParticles, Location, ParticleRot, ParticlesSize);
	}

	if (SurfaceEffect.ImpactDecal)
	{
		FRotator DecalRot(Rotation.Pitch, Rotation.Yaw, FMath::FRandRange(-180.f, 180.f));
		UGameplayStatics::SpawnDecalAtLocation(this, SurfaceEffect.ImpactDecal, DecalSize, Location, DecalRot, 5.f);
	}

	if (SurfaceEffect.ImpactSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, SurfaceEffect.ImpactSound, Location);
	}
}

void ADMProjectile_Single::ApplyProjectileDamage(const FHitResult& HitResult)
{
	if (IProjectileDamageInterface* DamageInterface = Cast<IProjectileDamageInterface>(GetOwner()))
	{
		DamageInterface->ApplyDamageEffect(HitResult, GetActorLocation());
	}
}
