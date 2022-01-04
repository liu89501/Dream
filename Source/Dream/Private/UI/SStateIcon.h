// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateUtils.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SStateIcon : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStateIcon)
	{}
		SLATE_ARGUMENT(UObject*, IconResource);
		SLATE_ARGUMENT(float, Duration);
		SLATE_ARGUMENT(int32, StackCount);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void RefreshState(int32 StackCount, float InDuration);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	TSharedPtr<STextBlock> StackCountText;

	TSharedPtr<class SProgressBar> DurationWidget;

	FLinearAnimation Duration;

	TSharedPtr<FSlateBrush> IconBrush;
};
