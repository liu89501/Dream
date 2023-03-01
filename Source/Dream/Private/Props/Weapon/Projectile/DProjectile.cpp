// Fill out your copyright notice in the Description page of Project Settings.

#include "DProjectile.h"
#include "DCharacterPlayer.h"
#include "DProjectileComponent.h"
#include "DMProjectSettings.h"
#include "PushModel.h"
#include "ShootWeapon.h"
#include "UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

#if DM_PROJECTILE_DEBUG

TAutoConsoleVariable<int32> DMProjectileCVar::CVarDebugProjectileTrace(
    TEXT("p.DMProjectileDebug"),
    0,
    TEXT("Whether to draw debug information.\n")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Cheat);

#endif

ADProjectile::ADProjectile()
    : ParticlesSize(FVector::OneVector)
    , DecalSize(20,5,5)
{
    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));

    SphereCollision->SetCollisionObjectType(Collision_ObjectType_Projectile);

    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetCollisionResponseToAllChannels(ECR_Block);
    SphereCollision->SetCollisionResponseToChannel(Collision_ObjectType_Projectile, ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(Collision_ObjectType_Weapon, ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    SphereCollision->OnComponentHit.AddDynamic(this, &ADProjectile::OnProjectileComponentHit);

    RootComponent = SphereCollision;

    TrailFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailFX"));
    TrailFX->SetupAttachment(SphereCollision);

    bReplicates = true;
    SetReplicatingMovement(true);

    Projectile = CreateDefaultSubobject<UDProjectileComponent>(TEXT("Projectile"));
    Projectile->MaxSpeed = 10000.f;
    Projectile->InitialSpeed = 10000.f;
    Projectile->ProjectileGravityScale = .6f;

    InitialLifeSpan = 5.f;
}

void ADProjectile::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    SphereCollision->MoveIgnoreActors.Add(GetOwner());

    if (GetOwner() != GetInstigator())
    {
        SphereCollision->MoveIgnoreActors.Add(GetInstigator());
    }

    if (IsReplicatingMovement() && GetLocalRole() != ROLE_Authority)
    {
        SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ADProjectile::OnRep_Impact()
{
}

void ADProjectile::OnProjectileComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                            const FHitResult& Hit)
{
    if (bImpact)
    {
        return;
    }

    SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Projectile->Deactivate();

    if (GetLocalRole() == ROLE_Authority)
    {
        if (IsReplicatingMovement())
        {
            bImpact = true;
            MARK_PROPERTY_DIRTY_FROM_NAME(ADProjectile, bImpact, this);
        }
        
        ApplyProjectileDamage(Hit);

        SetActorHiddenInGame(true);
        SetLifeSpan(3.f);
    }
    else if (!IsReplicatingMovement())
    {
        OnRep_Impact();
    }
}

void ADProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams Params;
    Params.bIsPushBased = true;
    
    DOREPLIFETIME_WITH_PARAMS_FAST(ADProjectile, bImpact, Params);
}
