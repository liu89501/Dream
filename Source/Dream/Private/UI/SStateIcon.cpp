// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SStateIcon.h"
#include "SImage.h"
#include "SlateImageBrush.h"
#include "SlateOptMacros.h"
#include "SProgressBar.h"
#include "Style/DreamStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SStateIcon::Construct(const FArguments& InArgs)
{
	IconBrush = MakeShared<FSlateImageBrush>(InArgs._IconResource, FVector2D(32,32));

	if (InArgs._Duration > 0)
	{
		Duration = InArgs._Duration;
		Duration.Activate();
	}

	const ISlateStyle& SlateStyle = FDreamStyle::Get();

	const FProgressBarStyle& ProgressBarStyle = SlateStyle.GetWidgetStyle<FProgressBarStyle>("Dream.DefaultProgressBarStyle");
	const FTextBlockStyle& TextBlockStyle = SlateStyle.GetWidgetStyle<FTextBlockStyle>("StateStackCountTextStyle");

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(IconBrush.Get())
		]
		+ SOverlay::Slot()
		[
			SAssignNew(DurationWidget, SProgressBar)
			.BarFillType(EProgressBarFillType::BottomToTop)
			.Style(&ProgressBarStyle)
			.Percent(0)
			.FillColorAndOpacity(FSlateColor(FLinearColor(0.5f,0.5f,0.5f,0.6f)))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
        .VAlign(VAlign_Bottom)
		[
			SAssignNew(StackCountText, STextBlock)
			.TextStyle(&TextBlockStyle)
			.Text(FText::FromString(FString::FromInt(InArgs._StackCount)))
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SStateIcon::RefreshState(int32 StackCount, float InDuration)
{
	Duration = InDuration;
	Duration.Reset();
	StackCountText->SetText(FText::FromString(FString::FromInt(StackCount)));
}

void SStateIcon::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (Duration.IsActive())
	{
		Duration.InterpConstantTo(InDeltaTime);
		
		DurationWidget->SetPercent(Duration.GetReverseProgress());

		if (Duration.IsCompleted())
		{
			Duration.Deactivate();
		}
	}
}