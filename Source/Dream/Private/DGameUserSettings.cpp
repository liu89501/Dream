// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameUserSettings.h"

const FLinearColor& UDGameUserSettings::GetThemeColor() const
{
	return ThemeColor;
}

void UDGameUserSettings::SetThemeColor(const FLinearColor& InThemeColor)
{
	ThemeColor = InThemeColor;
}

int32 UDGameUserSettings::GetMouseSensitivity() const
{
	return MouseSensitivity;
}

void UDGameUserSettings::SetMouseSensitivity(int32 InMouseSensitivity)
{
	MouseSensitivity = InMouseSensitivity;
}
