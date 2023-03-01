// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMProjectileTrackComponent.generated.h"

class ADProjectile;

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName="ProjectileTrack"))
class DREAM_API UDMProjectileTrackComponent : public UActorComponent
{
	GENERATED_BODY()

public:

    UDMProjectileTrackComponent();

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Traceable)
    float TrackCapsuleRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Traceable)
    float TrackCapsuleHalfHeight;
    
    /**
     * 开始捕获跟踪目标的等待时间
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Traceable)
    float StartCaptureTime;

public:

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void BeginPlay() override;

private:

	UPROPERTY()
	ADProjectile* OwningProjectile;
};
