// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/DProjectileTraceable.h"

#include "DProjectileComponent.h"
#include "DrawDebugHelpers.h"
#include "GenericTeamAgentInterface.h"
#include "ShootWeapon.h"
#include "GameFramework/ProjectileMovementComponent.h"

ADProjectileTraceable::ADProjectileTraceable()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADProjectileTraceable::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    FVector ActorLocation = GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(GetOwner());

    FHitResult Hit;
    FVector TraceEnd = ActorLocation + Projectile->Velocity.GetSafeNormal() * TrackCapsuleHalfHeight;
    
    bool bIsBlocking = GetWorld()->SweepSingleByObjectType(Hit, ActorLocation, TraceEnd, FQuat::Identity,
                                                           ECC_Pawn, FCollisionShape::MakeSphere(TrackCapsuleRadius),
                                                           QueryParams);

    if (bIsBlocking)
    {
        AActor* HitActor = Hit.GetActor();
        ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(GetWeapon()->GetOwner(), HitActor);
        
        if (Attitude == ETeamAttitude::Hostile && !HitActor->ActorHasTag(DreamActorTagName::Death))
        {
            SetActorTickEnabled(false);
            Projectile->HomingTargetComponent = HitActor->GetRootComponent();
        }
    }
}

void ADProjectileTraceable::BeginPlay()
{
    Super::BeginPlay();

    if (GetLocalRole() == ROLE_Authority)
    {
        if (FMath::IsNearlyZero(StartCaptureTime))
        {
            SetActorTickEnabled(true);
        }
        else
        {
            FTimerHandle Handle;
            GetWorldTimerManager().SetTimer(Handle, [this] { SetActorTickEnabled(true); }, StartCaptureTime, false);
        }
    }
}
