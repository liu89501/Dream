// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DProjectileComponent.h"

#include "DreamType.h"
#include "Kismet/KismetMathLibrary.h"


UDProjectileComponent::UDProjectileComponent()
		: HomingTrackingAngleYaw(60.f),
          HomingTrackingAnglePitch(60.f)
{
}

FVector UDProjectileComponent::ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	FRotator HomingDirection = UKismetMathLibrary::FindLookAtRotation(UpdatedComponent->GetComponentLocation(), HomingTargetComponent->GetComponentLocation());
	FRotator PrevDirection = InVelocity.ToOrientationRotator();

	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(HomingDirection, PrevDirection);

	if (UKismetMathLibrary::InRange_FloatFloat(Delta.Yaw, -HomingTrackingAngleYaw, HomingTrackingAngleYaw))
	{
		PrevDirection.Yaw += Delta.Yaw;
	}

	if (UKismetMathLibrary::InRange_FloatFloat(Delta.Pitch, -HomingTrackingAnglePitch, HomingTrackingAnglePitch))
	{
		PrevDirection.Pitch += Delta.Pitch;
	}

	return PrevDirection.Vector().GetSafeNormal() * HomingAccelerationMagnitude;
}
