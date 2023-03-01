// Fill out your copyright notice in the Description page of Project Settings.


#include "DMProjectileTrackComponent.h"
#include "DProjectile.h"
#include "DProjectileComponent.h"
#include "DrawDebugHelpers.h"
#include "DreamType.h"
#include "GenericTeamAgentInterface.h"
#include "ShootWeapon.h"
#include "GameFramework/ProjectileMovementComponent.h"

UDMProjectileTrackComponent::UDMProjectileTrackComponent()
    : TrackCapsuleRadius(200.f)
    , TrackCapsuleHalfHeight(8000.f)
    , StartCaptureTime(0.1f)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDMProjectileTrackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    FVector ActorLocation = OwningProjectile->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningProjectile);
    QueryParams.AddIgnoredActor(OwningProjectile->GetInstigator());

    FHitResult Hit;
    FVector TraceEnd = ActorLocation + OwningProjectile->GetVelocity().GetSafeNormal() * TrackCapsuleHalfHeight;
    
    bool bIsBlocking = GetWorld()->SweepSingleByObjectType(Hit, ActorLocation, TraceEnd, FQuat::Identity,
                                                           ECC_Pawn, FCollisionShape::MakeSphere(TrackCapsuleRadius),
                                                           QueryParams);

    if (bIsBlocking)
    {
        AActor* HitActor = Hit.GetActor();
        ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(OwningProjectile, HitActor);
        
        if (Attitude == ETeamAttitude::Hostile && !HitActor->ActorHasTag(DMActorTagName::Death))
        {
            SetComponentTickEnabled(false);
            OwningProjectile->Projectile->HomingTargetComponent = HitActor->GetRootComponent();
        }
    }
}

void UDMProjectileTrackComponent::BeginPlay()
{
    Super::BeginPlay();

    OwningProjectile = Cast<ADProjectile>(GetOwner());
    checkf(OwningProjectile, TEXT("Onwer Actor Can Only Be ADProjectile"));

    if (GetOwnerRole() == ROLE_Authority)
    {
        if (FMath::IsNearlyZero(StartCaptureTime))
        {
            SetComponentTickEnabled(true);
        }
        else
        {
            FTimerHandle Handle;
            GetWorld()->GetTimerManager().SetTimer(Handle, [this] { SetComponentTickEnabled(true); }, StartCaptureTime, false);
        }
    }
}
