// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Style/DialogSlateWidgetStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * 
 */
class SDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialog)
	{}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateDialog(EDialogType DialogType, const FText& InContent) const;

	void PlayFadeInAnim(float ViewportWidth);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:

	static void InitInstance();
	static void ResetInstance();

	static TSharedPtr<SDialog> SINGLETON;

private:

	TSharedPtr<STextBlock> Title;
	TSharedPtr<STextBlock> Content;

	TSharedPtr<SImage> Icon;
	TSharedPtr<FSlateBrush> LineBrush;

	float FadeInValue = 0;
	bool bPlayAnimation = false;
};
