// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamGameMode.h"
#include "DGameModeInLevel.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API ADGameModeInLevel : public ADreamGameMode
{
	GENERATED_BODY()

public:

	ADGameModeInLevel();

public:

	UFUNCTION(BlueprintCallable, Category=DGameMode)
	void RefreshStartSpot(FName PlayerStartTag);

protected:

	virtual bool GetPlayerRestartTransform(FTransform& RestartTransform) override;

private:

	int32 StartSpotIndex;
	
	UPROPERTY()
	TArray<class APlayerStart*> AvailableStartSpot;
};
