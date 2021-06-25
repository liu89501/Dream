// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DProjectile.h"
#include "DProjectileTraceable.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API ADProjectileTraceable : public ADProjectile
{
	GENERATED_BODY()

public:

    ADProjectileTraceable();

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

    virtual void Tick(float DeltaSeconds) override;

    virtual void BeginPlay() override;

private:

	UPROPERTY()
	AActor* CapturedActor;
};
