// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "DGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:

	UDGameUserSettings(): ThemeColor(FLinearColor(0.097667f, 0.643387f, 0.677083f))
	{
	}

	const FLinearColor& GetThemeColor() const
	{
		return ThemeColor;
	}

	void SetThemeColor(const FLinearColor& InThemeColor)
	{
		ThemeColor = InThemeColor;
	}

private:

	UPROPERTY(config)
	FLinearColor ThemeColor;

};
