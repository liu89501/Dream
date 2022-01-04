// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SDialog.h"
#include "SImage.h"
#include "SlateColorBrush.h"
#include "Style/DialogSlateWidgetStyle.h"
#include "Style/DreamStyle.h"
#include "SlateOptMacros.h"

TSharedPtr<SDialog> SDialog::SINGLETON;

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SDialog::Construct(const FArguments& InArgs)
{
	const FDialogSlateStyle& DialogStyle = FDreamStyle::Get().GetWidgetStyle<FDialogSlateStyle>("DialogStyle");

	LineBrush = MakeShared<FSlateColorBrush>(FLinearColor::White);
	LineBrush->SetImageSize(FVector2D(4.f, 4.f));

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(&DialogStyle.BackgroundBrush)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
                .Image(LineBrush.Get())
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(150.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(0.f)
				[
					SAssignNew(Icon, SImage)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(30.f, 0.f)
				.AutoWidth()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(0.f, 8.f)
					[
						SAssignNew(Title, STextBlock)
						.TextStyle(&DialogStyle.TitleTextStyle)
					]
					+ SVerticalBox::Slot()
					.Padding(0.f, 8.f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(Content, STextBlock)
						.TextStyle(&DialogStyle.ContentTextStyle)
					]
				]
			]

		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDialog::UpdateDialog(EDialogType DialogType, const FText& InContent) const
{
	if (Icon.IsValid())
	{
		const FDialogSlateStyle& DialogStyle = FDreamStyle::Get().GetWidgetStyle<FDialogSlateStyle>("DialogStyle");

		if (const FDialogInformation* IconInf = DialogStyle.Icons.Find(DialogType))
		{
			Icon->SetImage(&IconInf->DialogIcon);
			LineBrush->TintColor = IconInf->DialogIcon.TintColor;

			if (Title.IsValid())
			{
				Title->SetText(IconInf->DialogTitle);
			}
		}
	}

	if (Content.IsValid())
	{
		Content->SetText(InContent);
	}
}

void SDialog::PlayFadeInAnim(float ViewportWidth)
{
	bPlayAnimation = true;
	FadeInValue = ViewportWidth;
}

void SDialog::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bPlayAnimation)
	{
		FadeInValue = FMath::FInterpTo(FadeInValue, 0, InDeltaTime, 30.f);

		FTransform2D NewTransform;
		NewTransform.SetTranslation(FVector2D(-FadeInValue, 0));
		RenderTransform.Set(NewTransform);

		if (FMath::IsNearlyZero(FadeInValue))
		{
			bPlayAnimation = false;
		}
	}
}

void SDialog::InitInstance()
{
	SAssignNew(SINGLETON, SDialog);
}

void SDialog::ResetInstance()
{
	SINGLETON.Reset();
	SINGLETON = nullptr;
}
