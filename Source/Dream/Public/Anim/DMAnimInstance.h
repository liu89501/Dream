// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMCharacterType.h"
#include "Animation/AnimInstance.h"
#include "DMAnimInstance.generated.h"

UENUM()
enum class EMovementDirection : uint8
{
	Forward,
	Left,
	Right,
	Backward
};

USTRUCT(BlueprintType)
struct FAnimVelocityBlend
{
	GENERATED_BODY()

	FAnimVelocityBlend() : F(0.f), B(0.f), L(0.f), R(0.f)
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float F;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float B;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float L;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float R;
};

USTRUCT(BlueprintType)
struct FAnimLeanAmount
{
	GENERATED_BODY()

	FAnimLeanAmount() : LR(0.f), FB(0.f)
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ClampMin=-1, ClampMax=1))
	float LR;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ClampMin=-1, ClampMax=1))
	float FB;
};

/**
 * 
 */
UCLASS(Blueprintable)
class DREAM_API UDMAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UDMAnimInstance();

public:

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void UpdateAimingValues(float DeltaSeconds);
	
	void UpdateMovementValues(float DeltaSeconds);

	void UpdateRotationValues();

	bool ShouldMoveCheck() const;


protected:

	FRotator SmoothAimingRotation;
	FRotator AimingRotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Aiming")
	FVector2D AimingAngle;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Aiming")
	FVector2D SmoothAimingAngle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Aiming")
	FRotator SpineRotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Aiming")
	float LeftYawTime;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Aiming")
	float RightYawTime;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Aiming")
	float ForwardYawTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Aiming")
	float AimSweepTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float FYaw;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float BYaw;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float LYaw;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float RYaw;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	bool bIsMoving;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	bool bHasMovementInput;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	EMovementState MovementState;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	EMovementGait Gait;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	EMovementDirection MovementDirection;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	FAnimVelocityBlend VelocityBlend;

	UPROPERTY(EditAnywhere, Category="DMAnim|Grounded")
	float VelocityBlendInterpSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	FRotator ControlRotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	FVector Acceleration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	FVector RelativeAccelerationAmount;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	FAnimLeanAmount LeanAmount;

	UPROPERTY(EditAnywhere, Category="DMAnim|Grounded")
	float LeanAmountInterpSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float Speed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float WalkRunBlend;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float StrideBlend;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	float StandingPlayRate;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="DMAnim|Grounded")
	float AnimatedWalkSpeed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="DMAnim|Grounded")
	float AnimatedRunSpeed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="DMAnim|Grounded")
	float AnimatedSprintSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	bool RotateL;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DMAnim|Grounded")
	bool RotateR;
	
private:

	UPROPERTY(EditAnywhere, Category="DMAnim|BlendCurves")
	UCurveFloat* StrideBlendWalk;
	
	UPROPERTY(EditAnywhere, Category="DMAnim|BlendCurves")
	UCurveFloat* StrideBlendRun;

	bool PrevShouldMove;

	UPROPERTY()
	ACharacter* OwnerCharacter;
};


