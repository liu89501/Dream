// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DPlayerCameraManager.h"
#include "DCharacterPlayer.h"

ADPlayerCameraManager::ADPlayerCameraManager()
{
	ViewPitchMin = -60.f;
	ViewPitchMax = 70.f;
	EyeHeightInterpSpeed = 6.f;
}

void ADPlayerCameraManager::InitializeCameraManager(ADCharacterPlayer* InCharacterPlayer)
{
	CharacterPlayer = InCharacterPlayer;
	DefaultEyeHeight = CharacterPlayer->BaseEyeHeight;
}

void ADPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	if (CharacterPlayer)
	{
		DefaultEyeHeight = FMath::FInterpTo(DefaultEyeHeight, CharacterPlayer->BaseEyeHeight, DeltaTime, EyeHeightInterpSpeed);
		FRotator ControlRotation = CharacterPlayer->GetControlRotation() + RelativeRotation;
		FVector CharacterLocation = CharacterPlayer->GetMesh()->GetComponentLocation();
		OutVT.POV.Location = CharacterLocation + FVector(0,0,DefaultEyeHeight) + ControlRotation.RotateVector(RelativeLocation);
		OutVT.POV.Rotation = ControlRotation;
	}
	else
	{
		Super::UpdateViewTargetInternal(OutVT, DeltaTime);
	}
}

void ADPlayerCameraManager::SetRelativeLocation(const FVector& NewLocation)
{
	RelativeLocation = NewLocation;
}

void ADPlayerCameraManager::SetRelativeRotation(const FRotator& NewRotation)
{
	RelativeRotation = NewRotation;
}

FTransform ADPlayerCameraManager::GetRelativeTransform() const
{
	return FTransform(RelativeRotation, RelativeLocation);
}
