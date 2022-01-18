// Fill out your copyright notice in the Description page of Project Settings.

#include "DProjectile.h"
#include "DCharacterPlayer.h"
#include "DPlayerController.h"
#include "DProjectileComponent.h"
#include "DProjectSettings.h"
#include "DreamGameInstance.h"
#include "ShootWeapon.h"
#include "UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ADProjectile::ADProjectile()
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
    SphereCollision->MoveIgnoreActors.Add(GetInstigator());
}

void ADProjectile::OnRep_Impact()
{
    if (bRadialDamage)
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

        if (AShootWeapon* Weapon = GetWeapon())
        {
            if (ADCharacterPlayer* PlayerShooter = Weapon->GetOwningShooter())
            {
                if (PlayerShooter->IsLocallyControlled())
                {
                    float Distance = FVector::Distance(PlayerShooter->GetActorLocation(), GetActorLocation());
                    if (Distance < OuterCameraShakeRadius)
                    {
                        float Scale = FMath::Min((OuterCameraShakeRadius - Distance) / InnerCameraShakeRadius, 1.f);
                        PlayerShooter->GetPlayerController()->ClientStartCameraShake(ExplodedCameraShake, Scale);
                    }
                }
            }
        }
    }
    else
    {
        FHitResult HitInfo;

        FVector Location = GetActorLocation();

        FCollisionQueryParams CollisionQuery;
        CollisionQuery.bReturnPhysicalMaterial = true;
        CollisionQuery.AddIgnoredActor(this);

        UWorld* World = GetWorld();

        bool bHit = World->SweepSingleByChannel(HitInfo, Location, Location + 2.f, FQuat::Identity,
                                                ECC_Visibility,
                                                FCollisionShape::MakeSphere(
                                                    SphereCollision->GetScaledSphereRadius() + 8.f), CollisionQuery);

        EPhysicalSurface SurfaceType = bHit ? SurfaceType = HitInfo.PhysMaterial->SurfaceType : EPhysicalSurface::SurfaceType_Default;

        const FSurfaceImpactEffect& SurfaceEffect = UDProjectSettings::GetProjectSettings()->GetSurfaceImpactEffect(SurfaceType);

        FRotator Rotation = GetActorRotation();
        if (SurfaceEffect.ImpactParticles)
        {
            FRotator ParticleRot(Rotation.Pitch - 90.f, Rotation.Yaw, Rotation.Roll);
            UGameplayStatics::SpawnEmitterAtLocation(this, SurfaceEffect.ImpactParticles, Location, ParticleRot, ParticlesSize);
        }

        if (SurfaceEffect.ImpactDecal)
        {
            FRotator DecalRot(Rotation.Pitch, Rotation.Yaw, FMath::FRandRange(-180.f, 180.f));
            UGameplayStatics::SpawnDecalAtLocation(this, SurfaceEffect.ImpactDecal, DecalSize, Location, DecalRot, 10.f);
        }

        if (SurfaceEffect.ImpactSound)
        {
            UGameplayStatics::SpawnSoundAtLocation(this, SurfaceEffect.ImpactSound, Location);
        }
    }
}

AShootWeapon* ADProjectile::GetWeapon() const
{
    return Cast<AShootWeapon>(GetOwner());
}

void ADProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (GetLocalRole() != ROLE_Authority)
    {
        SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ADProjectile::OnProjectileComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                            const FHitResult& Hit)
{
    // Authority Only
    
    if (bImpact)
    {
        return;
    }
    
    bImpact = true;
    
    Projectile->StopSimulating(Hit);

    if (AShootWeapon* Weapon = GetWeapon())
    {
        if (bRadialDamage)
        {
            FRadialDamageProjectileInfo RadialDamage(GetActorLocation(), DamageRadius);
            Weapon->ApplyRadialDamage(RadialDamage);
        }
        else
        {
            Weapon->ApplyPointDamage(Hit);
        }
    }
    
    SetLifeSpan(2.f);
}

void ADProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADProjectile, bImpact);
}
