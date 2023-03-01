// Fill out your copyright notice in the Description page of Project Settings.

#include "DMShooterWeapon_Instant.h"
#include "DEnemyShooter.h"
#include "DGameplayStatics.h"
#include "DGameplayTags.h"
#include "DMEffect_ShootingDamage.h"

UDMShooterWeapon_Instant::UDMShooterWeapon_Instant()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDMShooterWeapon_Instant::OpenFire(AActor* Hostile)
{
	FVector ViewLocation = OwnerAI->GetPawnViewLocation();
	FVector HostileLocation = Hostile->GetActorLocation();
	
	FVector TraceEnd = HostileLocation + (HostileLocation - ViewLocation).GetSafeNormal() * 100.f;
				
	GetWorld()->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateUObject(this, &UDMShooterWeapon_Instant::AttemptApplyDamage, TraceEnd));
	
}

void UDMShooterWeapon_Instant::AttemptApplyDamage(FVector TraceEnd)
{
	FVector ViewLocation = OwnerAI->GetPawnViewLocation();
	
	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerAI);
	
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility, QueryParams);

	if (HitResult.bBlockingHit)
	{
		UDGameplayStatics::ApplyGameplayEffect(OwnerAI->AbilitySystem,
											   HitResult.GetActor(),
											   UDMEffect_ShootingDamage::StaticClass(),
											   1.f,
											   false,
											   DamageType);
	}

	MulticastFiringFX(HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEnd);
}


void UDMShooterWeapon_Instant::MulticastFiringFX_Implementation(const FVector_NetQuantize10& ImpactPoint)
{
	PlayAnimAndFX();
	UDGameplayStatics::SpawnWeaponTrailParticles(this, TrailVfx, OwnerAI->GetWeaponMuzzleLocation(), ImpactPoint);
}
