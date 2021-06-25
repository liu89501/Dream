// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShake.h"
#include "DProjectile.generated.h"

/**
 * 
 */
UCLASS()
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

    /**
     * 此发射物是否为范围伤害
     */
    UPROPERTY(EditAnywhere, Category=Projectile)
    bool bRadialDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (EditCondition = "bRadialDamage"))
    class UParticleSystem* HitParticles;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (EditCondition = "bRadialDamage"))
    class USoundCue* HitSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (EditCondition = "bRadialDamage"))
    float DamageRadius;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (EditCondition = "bRadialDamage"))
    TSubclassOf<UMatineeCameraShake> ExplodedCameraShake;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (EditCondition = "bRadialDamage"))
    float InnerCameraShakeRadius;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (EditCondition = "bRadialDamage"))
    float OuterCameraShakeRadius;

protected:

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Impact)
    bool bImpact;

protected:

    UFUNCTION()
    virtual void OnRep_Impact();

    class AShootWeapon* GetWeapon() const;

    virtual void BeginPlay() override;

    UFUNCTION()
    void OnProjectileComponentHit(UPrimitiveComponent* HitComponent,
                                  AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp,
                                  FVector NormalImpulse,
                                  const FHitResult& Hit);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
