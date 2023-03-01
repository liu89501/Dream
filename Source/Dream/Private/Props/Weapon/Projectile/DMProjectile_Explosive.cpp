// Fill out your copyright notice in the Description page of Project Settings.

#include "DMProjectile_Explosive.h"
#include "DreamType.h"
#include "ProjectileDamageInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void ADMProjectile_Explosive::OnRep_Impact()
{
	if (HitParticles)
	{
		FRotator Rotation = GetActorRotation();
		FRotator ParticleRot(Rotation.Pitch - 90, Rotation.Yaw, Rotation.Roll);
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, GetActorLocation(), ParticleRot, ParticlesSize);
	}

	if (HitSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, HitSound, GetActorLocation());
	}
}

void ADMProjectile_Explosive::ApplyProjectileDamage(const FHitResult& HitResult)
{
	IProjectileDamageInterface* DamageInterface = Cast<IProjectileDamageInterface>(GetOwner());

	if (DamageInterface)
	{
		FVector ActorLocation = GetActorLocation();

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		if (GetOwner() != GetInstigator())
		{
			QueryParams.AddIgnoredActor(GetInstigator());
		}

		TArray<FHitResult> Hits;
		GetWorld()->SweepMultiByChannel(Hits, ActorLocation, ActorLocation, FQuat::Identity,
                                        ECC_Visibility, FCollisionShape::MakeSphere(DamageRadius), QueryParams);


		for (FArrayDistinctIterator<FHitResult, FHitResultKeyFuncs> It(Hits); It; ++It)
		{
			DamageInterface->ApplyDamageEffect(*It, ActorLocation);
		}
	}
}
