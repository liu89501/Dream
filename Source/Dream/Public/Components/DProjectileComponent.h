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

	/** 激活追踪效果时此设置才有效 表示跟踪过程中与目标的夹角最大只能为多少 超过了此值追踪效果会失效 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (ClampMin=0, ClampMax=180), Category=Advanced)
	float LimitAngle;

	
public:

	virtual FVector ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const override;
};
