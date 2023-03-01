// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DProjectile.generated.h"

#define DM_PROJECTILE_DEBUG !(UE_BUILD_TEST || UE_BUILD_SHIPPING)

#if DM_PROJECTILE_DEBUG

struct DMProjectileCVar
{
    static TAutoConsoleVariable<int32> CVarDebugProjectileTrace;
};

#endif

/**
 * 
 */
UCLASS(Abstract)
class DREAM_API ADProjectile : public AActor
{
    GENERATED_BODY()

public:

    ADProjectile();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
    class USphereComponent* SphereCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
    class UParticleSystemComponent* TrailFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
    class UDProjectileComponent* Projectile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Adjustment")
    FVector ParticlesSize;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Adjustment")
    FVector DecalSize;

protected:

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Impact)
    bool bImpact;

protected:

    virtual void PostInitializeComponents() override;

    UFUNCTION()
    virtual void OnRep_Impact();

    UFUNCTION()
    virtual void OnProjectileComponentHit(UPrimitiveComponent* HitComponent,
                                  AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp,
                                  FVector NormalImpulse,
                                  const FHitResult& Hit);

    virtual void ApplyProjectileDamage(const FHitResult& HitResult) {}

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
