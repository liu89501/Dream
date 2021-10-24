// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "DGameUserSettings.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class DREAM_API UDGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:

	UDGameUserSettings()
	{
	}

	UFUNCTION(BlueprintPure, Category=DreamUserSettings)
	const FLinearColor& GetThemeColor() const;

	UFUNCTION(BlueprintCallable, Category=DreamUserSettings)
	void SetThemeColor(const FLinearColor& InThemeColor);

	UFUNCTION(BlueprintPure, Category=DreamUserSettings)
	int32 GetMouseSensitivity() const;
	
	FORCEINLINE int32 GetCalculatedMouseSensitivity() const
	{
		// MouseSensitivity 1 ~ 15   *   3
		return MouseSensitivity * 3;
	}
	

	UFUNCTION(BlueprintCallable, Category=DreamUserSettings)
	void SetMouseSensitivity(int32 InMouseSensitivity);

private:

	UPROPERTY(config)
	FLinearColor ThemeColor;

	UPROPERTY(config)
	int32 MouseSensitivity;

};
