// Fill out your copyright notice in the Description page of Project Settings.

#include "DMShooterWeapon_SuggestProjectile.h"
#include "DEnemyShooter.h"
#include "DProjectile.h"
#include "DProjectileComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void UDMShooterWeapon_SuggestProjectile::OpenFire(AActor* Hostile)
{
	if (ProjectileClass && Hostile)
	{
		FTransform Transform;
		
		FVector Location = OwnerAI->GetPawnViewLocation();

		FRandomStream RandomStream(FMath::Rand());

		FVector HostileLocation;
		if (ACharacter* HostileCharacter = Cast<ACharacter>(Hostile))
		{
			HostileLocation = HostileCharacter->GetMesh()->GetComponentLocation();
		}
		else
		{
			HostileLocation = Hostile->GetActorLocation();
		}

		Transform.SetRotation(RandomStream.VRandCone(HostileLocation - Location,
                                             HorizontalConeHalfAngleRad, VerticalConeHalfAngleRad).Rotation().Quaternion());

		Transform.SetTranslation(Location);

#if DM_PROJECTILE_DEBUG

		if (DMProjectileCVar::CVarDebugProjectileTrace.GetValueOnAnyThread() == 1)
		{
			DrawDebugSphere(GetWorld(), Transform.GetLocation(), 60.f, 12, FColor::Red, false, 5.f, 0, 1.f);
		}
		
#endif

		ADProjectile* Projectile = GetWorld()->SpawnActorDeferred<ADProjectile>(ProjectileClass,
																				Transform, OwnerAI, OwnerAI,
																				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		
		FVector TossVelocity;
		UGameplayStatics::SuggestProjectileVelocity(this,
												 TossVelocity,
													Location,
													HostileLocation,
													Projectile->Projectile->InitialSpeed,
													bFavorHighArc,
													0.f,
													Projectile->Projectile->GetGravityZ(),
													ESuggestProjVelocityTraceOption::DoNotTrace);

		/*UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, TossVelocity,
															  Location,
															  HostileLocation,
															  )*/


		Transform.SetRotation(TossVelocity.Rotation().Quaternion());
		Projectile->FinishSpawning(Transform);

		MulticastAnimAndFX();
	}
}
