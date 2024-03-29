// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SSubtitle.h"
#include "SlateOptMacros.h"
#include "Style/DreamStyle.h"

FText SSubtitle::EMPTY;

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSubtitle::Construct(const FArguments& InArgs)
{
	DisplayMode = InArgs._DisplayMode;

	DisplayTime = InArgs._DisplayTime;
	FadeOut = InArgs._FadeOutTime;

	const FTextBlockStyle& SubtitleStyle = FDreamStyle::Get().GetWidgetStyle<FTextBlockStyle>("SubtitleTextStyle");

	SetCanTick(false);

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		.Padding(100.f, 80.f)
		[
			SAssignNew(Subtitle, STextBlock)
            .TextStyle(&SubtitleStyle)
            .AutoWrapText(true)
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSubtitle::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (DisplayMode == EDisplayMode::OnePerFrame)
	{
		if (SubtitleIdx.IsActive())
		{
			uint32 NewIdx = SubtitleIdx.Increment();
			Subtitle->SetText(FText::FromString(PendingDisplaySubtitle.Left(NewIdx)));

			if (SubtitleIdx.IsCompleted())
			{
				SubtitleIdx.Toggle();
				DisplayTime.Activate();
				PendingDisplaySubtitle.Empty();
			}
		}
	}

	if (DisplayTime.IsActive())
	{
		DisplayTime.InterpConstantTo(InDeltaTime);

		if (DisplayTime.IsCompleted())
		{
			DisplayTime.Deactivate();
			DisplayTime.Reset();
			FadeOut.Activate();
		}
	}

	if (FadeOut.IsActive())
	{
		FadeOut.InterpConstantTo(InDeltaTime);
		SetRenderOpacity(FadeOut.GetReverseProgress());
		
		if (FadeOut.IsCompleted())
		{
			HiddenSubtitle();
			FadeOut.Reset();
			FadeOut.Deactivate();
		}
	}
}

void SSubtitle::DisplaySubtitle(const FText& SubtitleText)
{
	Subtitle->SetVisibility(EVisibility::SelfHitTestInvisible);
	
	if (DisplayMode == EDisplayMode::Direct)
	{
		DisplayTime.Activate();
		Subtitle->SetText(SubtitleText);
	}
	else
	{
		PendingDisplaySubtitle = SubtitleText.ToString();
		SubtitleIdx.Initialize(PendingDisplaySubtitle.Len());
        SubtitleIdx.Toggle();
	}
}

void SSubtitle::HiddenSubtitle() const
{
	Subtitle->SetText(EMPTY);
	Subtitle->SetVisibility(EVisibility::Collapsed);
}
