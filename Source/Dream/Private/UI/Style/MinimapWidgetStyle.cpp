// Fill out your copyright notice in the Description page of Project Settings.

#include "MinimapWidgetStyle.h"

FMinimapStyle::FMinimapStyle()
{
}

FMinimapStyle::~FMinimapStyle()
{
}

const FName FMinimapStyle::TypeName(TEXT("FMinimapStyle"));

const FMinimapStyle& FMinimapStyle::GetDefault()
{
	static FMinimapStyle Default;
	return Default;
}

void FMinimapStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
	OutBrushes.Add(&WarningOuterBrush);
	OutBrushes.Add(&WarningMiddleBrush);
	OutBrushes.Add(&WarningCenterBrush);
	OutBrushes.Add(&PointerIconBrush);
	OutBrushes.Add(&BackgroundBrush);
}

