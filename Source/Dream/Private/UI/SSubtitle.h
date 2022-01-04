// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SlateUtils.h"
#include "Widgets/SCompoundWidget.h"

enum class EDisplayMode : uint8
{
	OnePerFrame,
	All
};

/**
 * 
 */
class SSubtitle : public SCompoundWidget
{
	
public:
	
	SLATE_BEGIN_ARGS(SSubtitle)
		: _DisplayMode(EDisplayMode::All)
	{}
		SLATE_ARGUMENT(EDisplayMode, DisplayMode);
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void DisplaySubtitle(const FText& SubtitleText);
	
	void HiddenSubtitle() const;

private:

	TSharedPtr<STextBlock> Subtitle;

	FString PendingDisplaySubtitle;
	FNumberCounter SubtitleIdx;
	
	EDisplayMode DisplayMode;

	static FText EMPTY;
};
