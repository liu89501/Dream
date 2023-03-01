// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundCue.h"
#include "DMUISoundSet.generated.h"


/**
 * 
 */
UCLASS(BlueprintType)
class DREAM_API UDMUISoundSet : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * 普通按钮的声音
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* ClickSoundNormal;

	/**
	 *对于那种需要长按的按钮的声音
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* ClickSoundHold;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* HoverSoundStyle1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* HoverSoundStyle2;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* Success;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* Error;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* Lock;
	
};
