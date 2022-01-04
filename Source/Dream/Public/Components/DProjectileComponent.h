// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DProjectileComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=Movement, meta=(BlueprintSpawnableComponent))
class DREAM_API UDProjectileComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:

	UDProjectileComponent();

	/** 激活追踪效果时此设置才有效 表示跟踪过程中与目标的Yaw夹角最大只能为多少 超过了此值追踪效果会失效 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Advanced)
	float HomingTrackingAngleYaw;

	/** 激活追踪效果时此设置才有效 表示跟踪过程中与目标的Yaw夹角最大只能为多少 超过了此值追踪效果会失效 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Advanced)
	float HomingTrackingAnglePitch;


	virtual FVector ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const override;
};
