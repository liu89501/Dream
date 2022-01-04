// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Style/InteractiveWidgetStyle.h"

FInteractiveStyle::FInteractiveStyle(): InteractiveSoundCue(nullptr)
{
}

FInteractiveStyle::~FInteractiveStyle()
{
}

const FName FInteractiveStyle::TypeName(TEXT("FInteractiveStyle"));

const FInteractiveStyle& FInteractiveStyle::GetDefault()
{
	static FInteractiveStyle Default;
	return Default;
}

void FInteractiveStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them

	OutBrushes.Add(&KeyBackgroundBrush);
	OutBrushes.Add(&PressedProgressBrush);
	KeyFontStyle.GetResources(OutBrushes);
	HintFontStyle.GetResources(OutBrushes);
}

