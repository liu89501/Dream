// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "DPlayerCameraManager.generated.h"

class ADCharacterPlayer;

/**
 * 
 */
UCLASS()
class DREAM_API ADPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	ADPlayerCameraManager();

public:

	virtual void InitializeCameraManager(ADCharacterPlayer* InCharacterPlayer);

	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;

	void SetRelativeLocation(const FVector& NewLocation);
	void SetRelativeRotation(const FRotator& NewRotation);

	FTransform GetRelativeTransform() const;

	FVector GetRelativeLocation() const
	{
		return RelativeLocation;
	}

protected:

	/** Actor眼睛高度变化插值速度 */
	UPROPERTY(EditAnywhere, Category=DreamrCameraManager)
	float EyeHeightInterpSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category=DreamrCameraManager)
	ADCharacterPlayer* CharacterPlayer;

	UPROPERTY(BlueprintReadOnly, Category=DreamrCameraManager)
	float DefaultEyeHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DreamrCameraManager)
	FVector RelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DreamrCameraManager)
	FRotator RelativeRotation;
};
