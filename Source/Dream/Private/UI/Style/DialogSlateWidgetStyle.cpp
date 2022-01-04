// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogSlateWidgetStyle.h"

FDialogSlateStyle::FDialogSlateStyle()
{
}

FDialogSlateStyle::~FDialogSlateStyle()
{
}

const FName FDialogSlateStyle::TypeName(TEXT("FDialogSlateStyle"));

const FDialogSlateStyle& FDialogSlateStyle::GetDefault()
{
	static FDialogSlateStyle Default;
	return Default;
}

void FDialogSlateStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
	OutBrushes.Add(&BackgroundBrush);
	TitleTextStyle.GetResources(OutBrushes);
	ContentTextStyle.GetResources(OutBrushes);

	for (const TTuple<EDialogType, FDialogInformation>& Pair : Icons)
	{
		OutBrushes.Add(&Pair.Value.DialogIcon);
	}
}

