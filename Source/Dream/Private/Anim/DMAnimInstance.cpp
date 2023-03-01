// Fill out your copyright notice in the Description page of Project Settings.

#include "DMAnimInstance.h"
#include "DMCharacterInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#define Execute_AnimInterface(Object, Func, ...) \
	if (Object->GetClass()->ImplementsInterface(IDMCharacterInterface::UClassType::StaticClass())) \
	{ \
		IDMCharacterInterface::Execute_##Func(Object, __VA_ARGS__); \
	} \
	else if (IDMCharacterInterface* AnimationInterface = Cast<IDMCharacterInterface>(Object)) \
	{ \
		AnimationInterface->##Func(__VA_ARGS__); \
	} \

UDMAnimInstance::UDMAnimInstance()
	: VelocityBlendInterpSpeed(12.f)
	, LeanAmountInterpSpeed(4.f)
	, WalkRunBlend(0.f)
{
}

void UDMAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());

	if (OwnerCharacter)
	{
		Execute_AnimInterface(OwnerCharacter, GetDefaultValues, AnimatedWalkSpeed, AnimatedRunSpeed, AnimatedSprintSpeed);
	}
}

void UDMAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (!OwnerCharacter)
	{
		return;
	}

	Execute_AnimInterface(
		OwnerCharacter,
		GetBasicValues,
		ControlRotation,
		Acceleration,
		AimingRotation,
		Speed,
		bIsMoving,
		bHasMovementInput
	);
	
	Execute_AnimInterface(
		OwnerCharacter,
		GetCharacterStates,
		MovementState,
		Gait
	);

	UpdateAimingValues(DeltaSeconds);
	
	bool bShouldMoveCheck = ShouldMoveCheck();

	if (bShouldMoveCheck)
	{
		UpdateMovementValues(DeltaSeconds);

		UpdateRotationValues();

		if (PrevShouldMove)
		{
			RotateL = false;
			RotateR = false;
		}
	}

	PrevShouldMove = bShouldMoveCheck;
}

void UDMAnimInstance::UpdateAimingValues(float DeltaSeconds)
{
	FRotator ActorRotation = OwnerCharacter->GetActorRotation();
	
	SmoothAimingRotation = FMath::RInterpTo(SmoothAimingRotation, AimingRotation, DeltaSeconds, 10.f);
	FRotator DeltaRot = (AimingRotation - ActorRotation).GetNormalized();
	AimingAngle = FVector2D(DeltaRot.Yaw, DeltaRot.Pitch);

	FRotator SmoothDeltaRot = (SmoothAimingRotation - ActorRotation).GetNormalized();
	SmoothAimingAngle = FVector2D(SmoothDeltaRot.Yaw, SmoothDeltaRot.Pitch);

	AimSweepTime = FMath::GetMappedRangeValueClamped(FVector2D(-90.f, 90.f), FVector2D(1.f, 0.f), AimingAngle.Y);

	SpineRotation.Yaw = AimingAngle.X / 4.f;

	float AbsSmoothAimingAngleX = FMath::Abs(SmoothAimingAngle.X);
	LeftYawTime = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 180.f), FVector2D(0.5f, 0.f), AbsSmoothAimingAngleX);
	RightYawTime = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 180.f), FVector2D(0.5f, 1.f), AbsSmoothAimingAngleX);
	ForwardYawTime = FMath::GetMappedRangeValueClamped(FVector2D(-180.f, 180.f), FVector2D(0.f, 1.f), SmoothAimingAngle.X);
}

void UDMAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	FRotator ActorRotation = OwnerCharacter->GetActorRotation();
	FVector Velocity = OwnerCharacter->GetVelocity();
	
	FVector VelocityNormal = Velocity.GetSafeNormal2D(KINDA_SMALL_NUMBER);
	FVector RelativeVelocity = ActorRotation.UnrotateVector(VelocityNormal);

	float Sum = FMath::Abs(RelativeVelocity.X) + FMath::Abs(RelativeVelocity.Y);

	RelativeVelocity /= Sum;

	VelocityBlend.F = FMath::FInterpTo(VelocityBlend.F, FMath::Clamp(RelativeVelocity.X, 0.f, 1.f), DeltaSeconds, VelocityBlendInterpSpeed);
	VelocityBlend.B = FMath::FInterpTo(VelocityBlend.B, FMath::Clamp(RelativeVelocity.X, -1.f, 0.f), DeltaSeconds, VelocityBlendInterpSpeed);
	VelocityBlend.L = FMath::FInterpTo(VelocityBlend.L, FMath::Clamp(RelativeVelocity.Y, 0.f, 1.f), DeltaSeconds, VelocityBlendInterpSpeed);
	VelocityBlend.R = FMath::FInterpTo(VelocityBlend.R, FMath::Clamp(RelativeVelocity.Y, -1.f, 0.f), DeltaSeconds, VelocityBlendInterpSpeed);

	if ((Acceleration | Velocity) > 0) // Dot
	{
		float MaxAcceleration = OwnerCharacter->GetCharacterMovement()->GetMaxAcceleration();
		FVector ClampedAcceleration = Acceleration.GetClampedToMaxSize(MaxAcceleration);
		RelativeAccelerationAmount = ActorRotation.UnrotateVector(ClampedAcceleration / MaxAcceleration);
	}
	else
	{
		float MaxBrakingDeceleration = OwnerCharacter->GetCharacterMovement()->GetMaxBrakingDeceleration();
		FVector ClampedDeceleration = Acceleration.GetClampedToMaxSize(MaxBrakingDeceleration);
		RelativeAccelerationAmount = ActorRotation.UnrotateVector(ClampedDeceleration / MaxBrakingDeceleration);
	}

	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, RelativeAccelerationAmount.X, DeltaSeconds, LeanAmountInterpSpeed);
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, RelativeAccelerationAmount.Y, DeltaSeconds, LeanAmountInterpSpeed);

	WalkRunBlend = Gait == EMovementGait::Walking ? 0.f : 1.f;

	float Alpha = FMath::Clamp(GetCurveValue("Weight_Gait") - 1.f, 0.f, 1.f);
	StrideBlend = FMath::Lerp(StrideBlendWalk->GetFloatValue(Speed), StrideBlendRun->GetFloatValue(Speed), Alpha);

	float WalkRunLerp = FMath::Lerp(Speed / AnimatedWalkSpeed, Speed / AnimatedRunSpeed, Alpha);

	float SprintAlpha = FMath::Clamp(GetCurveValue("Weight_Gait") - 2.f, 0.f, 1.f);
	StandingPlayRate = FMath::Clamp(FMath::Lerp(WalkRunLerp, Speed / AnimatedSprintSpeed, SprintAlpha) / StrideBlend, 0.f, 3.f);
}

void UDMAnimInstance::UpdateRotationValues()
{
	if (Gait == EMovementGait::Sprinting)
	{
		MovementDirection = EMovementDirection::Forward;
	}
	else
	{
		float Angle = (OwnerCharacter->GetVelocity().Rotation() - AimingRotation).GetNormalized().Yaw;

		// Buffer 5.f
		if (UKismetMathLibrary::InRange_FloatFloat(Angle, -75.f, 75.f))
		{
			MovementDirection = EMovementDirection::Forward;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(Angle, 75.f, 115.f))
		{
			MovementDirection = EMovementDirection::Right;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(Angle, -115.f, -75.f))
		{
			MovementDirection = EMovementDirection::Left;
		}
		else
		{
			MovementDirection = EMovementDirection::Backward;
		}
	}
}

bool UDMAnimInstance::ShouldMoveCheck() const
{
	return (bIsMoving && bHasMovementInput) || Speed >= AnimatedWalkSpeed;
}
