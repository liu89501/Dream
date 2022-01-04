// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerHUDStyle.h"

FPlayerHUDWidgetStyle::FPlayerHUDWidgetStyle(): HurtAnimCurve(nullptr)
{
}

FPlayerHUDWidgetStyle::~FPlayerHUDWidgetStyle()
{
}

const FName FPlayerHUDWidgetStyle::TypeName(TEXT("FPlayerHUDWidgetStyle"));

const FPlayerHUDWidgetStyle& FPlayerHUDWidgetStyle::GetDefault()
{
	static FPlayerHUDWidgetStyle Default;
	return Default;
}

void FPlayerHUDWidgetStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them

	AmmoTextStyle.GetResources(OutBrushes);
	MagazineTextStyle.GetResources(OutBrushes);
	HealthBarStyle.GetResources(OutBrushes);
	HealthBarBackgroundStyle.GetResources(OutBrushes);
	MagazineStyle.GetResources(OutBrushes);
	OutBrushes.Add(&HitMarkBrush);
	OutBrushes.Add(&HurtBrush);
}

