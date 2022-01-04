// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FGuideElement
{
	const FSlateBrush* Brush;
	FVector Location;
};

/**
 * 
 */
class SGuide : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGuide)
		:_Padding(FVector2D(8, 8))
	{}
		SLATE_ATTRIBUTE(TArray<FGuideElement>, Elements);
	
		SLATE_ARGUMENT(APlayerController*, PlayerOwner);
	
		SLATE_ARGUMENT(FVector2D, Padding);
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override;

private:

	TAttribute<TArray<FGuideElement>> Elements;

	TWeakObjectPtr<APlayerController> PlayerOwner;

	FVector2D Padding;
};
