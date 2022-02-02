// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DProjectileComponent.h"
#include "Kismet/KismetMathLibrary.h"


UDProjectileComponent::UDProjectileComponent()
	: LimitAngle(135.f)
{
}

FVector UDProjectileComponent::ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	FVector NewDirection = HomingTargetComponent->GetComponentLocation() - UpdatedComponent->GetComponentLocation();

	if (LimitAngle > 0.f)
	{
		FRotator NewRotation = NewDirection.Rotation();
		FRotator CurrentRotation = InVelocity.Rotation();
		FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(NewRotation, CurrentRotation);

		if (UKismetMathLibrary::InRange_FloatFloat(Delta.Yaw, -LimitAngle, LimitAngle))
		{
			CurrentRotation.Yaw += Delta.Yaw;
		}

		if (UKismetMathLibrary::InRange_FloatFloat(Delta.Pitch, -LimitAngle, LimitAngle))
		{
			CurrentRotation.Pitch += Delta.Pitch;
		}

		NewDirection = CurrentRotation.Vector();
	}

	return NewDirection.GetSafeNormal() * HomingAccelerationMagnitude;
}
