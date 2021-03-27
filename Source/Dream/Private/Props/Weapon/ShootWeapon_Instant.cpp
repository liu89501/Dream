// Fill out your copyright notice in the Description page of Project Settings.

#include "Props/Weapon/ShootWeapon_Instant.h"
#include "DreamGameInstance.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void AShootWeapon_Instant::BeginPlay()
{
	Super::BeginPlay();
}

void AShootWeapon_Instant::HandleSpawnAmmo(const FHitResult& HitResult)
{
	bool bAuthority = GetLocalRole() == ROLE_Authority;

	if (!bAuthority)
	{
		if (BulletTrace)
		{
			if (FVector::Distance(HitResult.ImpactPoint, HitResult.TraceStart) > 300.f)
			{
				UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), BulletTrace, HitResult.TraceStart, FRotator::ZeroRotator,
                FVector::ZeroVector, true, true, ENCPoolMethod::AutoRelease);

				NC->SetVectorParameter(TraceParamName, HitResult.ImpactPoint);
			}
		}
	}

	if (HitResult.bBlockingHit)
	{
		if (bAuthority)
		{
			ApplyPointDamage(HitResult);
		}
		else
		{
			UDreamGameInstance* GI = Cast<UDreamGameInstance>(GetGameInstance());
			EPhysicalSurface PhysicalSurface = HitResult.PhysMaterial.IsValid() ? 
                HitResult.PhysMaterial->SurfaceType.GetValue() : EPhysicalSurface::SurfaceType_Default;

			const FSurfaceImpactEffect& SurfaceEffect = GI->GetSurfaceImpactEffect(PhysicalSurface);

			FRotator OrientationRotator = HitResult.ImpactNormal.ToOrientationRotator();
			OrientationRotator.Pitch -= 90.f;

			UGameplayStatics::SpawnEmitterAtLocation(this, SurfaceEffect.ImpactParticles, HitResult.ImpactPoint, 
                                                     OrientationRotator, FVector::OneVector, true, EPSCPoolMethod::AutoRelease);
					
			UGameplayStatics::SpawnSoundAtLocation(this, SurfaceEffect.ImpactSound, HitResult.ImpactPoint);
		}
	}
}
