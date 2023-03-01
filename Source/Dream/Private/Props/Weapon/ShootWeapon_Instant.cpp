// Fill out your copyright notice in the Description page of Project Settings.

#include "Props/Weapon/ShootWeapon_Instant.h"
#include "DGameplayStatics.h"
#include "DMProjectSettings.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AShootWeapon_Instant::AShootWeapon_Instant()
	: DecalSize(FVector(20,5,5))
{
	
}

void AShootWeapon_Instant::BeginPlay()
{
	Super::BeginPlay();
}

void AShootWeapon_Instant::HandleSpawnProjectile(const FHitResult& ActualHitResult)
{
	if (ActualHitResult.bBlockingHit)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			ApplyDamageEffect(ActualHitResult, GetOwner()->GetActorLocation());
		}
		else
		{
			UDGameplayStatics::SpawnWeaponTrailParticles(this, TrailVfx, ActualHitResult.TraceStart,
														 ActualHitResult.bBlockingHit ? ActualHitResult.ImpactPoint : ActualHitResult.TraceEnd);
			
			if (ActualHitResult.bBlockingHit)
			{
				EPhysicalSurface PhysicalSurface = ActualHitResult.PhysMaterial.IsValid() ? 
                                ActualHitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType_Default;

				const FSurfaceImpactEffect& SurfaceEffect = GSProject->GetSurfaceImpactEffect(PhysicalSurface);
				
				FRotator NormalRot = ActualHitResult.ImpactNormal.Rotation();
				
				UGameplayStatics::SpawnEmitterAtLocation(this,
														 SurfaceEffect.ImpactParticles,
														 ActualHitResult.ImpactPoint, 
														 FRotator(NormalRot.Pitch - 90.f, NormalRot.Yaw, NormalRot.Roll),
														 FVector::OneVector,
														 true,
														 EPSCPoolMethod::AutoRelease);

				UGameplayStatics::SpawnSoundAtLocation(this, SurfaceEffect.ImpactSound, ActualHitResult.ImpactPoint);

				if (SurfaceEffect.ImpactDecal)
				{
					UGameplayStatics::SpawnDecalAtLocation(this,
                                                       SurfaceEffect.ImpactDecal,
                                                       DecalSize,
                                                       ActualHitResult.ImpactPoint,
                                                       FRotator(NormalRot.Pitch, NormalRot.Yaw + 180.f, NormalRot.Roll),
                                                       5.f);
				}
			}
		}
	}
}

void AShootWeapon_Instant::GetActualProjectileHitResult(const FHitResult& ViewHitResult, FHitResult& ActualHitResult) const
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());
	QueryParams.bReturnPhysicalMaterial = true;
	
	FVector MuzzleLoc;
	FRotator MuzzleRot;
	GetMuzzlePoint(MuzzleLoc, MuzzleRot);

	const FVector& ActualTraceEnd = ViewHitResult.bBlockingHit ?
									ViewHitResult.ImpactPoint + MuzzleRot.Vector().GetSafeNormal() * 50.f :
									ViewHitResult.TraceEnd;
	
	GetWorld()->LineTraceSingleByChannel(ActualHitResult, MuzzleLoc, ActualTraceEnd, ECC_Visibility, QueryParams);

#if ENABLE_WEAPON_DEBUG

	if (DebugWeaponCVar::DebugCVar.GetValueOnAnyThread() == 1)
	{
		UKismetSystemLibrary::DrawDebugLine(this, MuzzleLoc, ActualTraceEnd, FLinearColor::Yellow, 5.f, 1.f);

		if (ActualHitResult.bBlockingHit)
		{
			UKismetSystemLibrary::DrawDebugPoint(this, ActualHitResult.ImpactPoint, 14.f, FLinearColor::Green, 5.f);
		}
	}
	
#endif
	
}
