
#include "SDialog.h"
#include "Engine.h"

#define LOCTEXT_NAMESPACE "Dream.Dialog"

void SDialog::Construct(const FArguments& InArgs)
{
	FButtonStyle DefultButtonStyle;
	FSlateColorBrush Normal(FLinearColor(0, 1, 1, 0.8f));
	FSlateColorBrush Hover(FLinearColor(0, 1, 1, 0.6f));
	FSlateColorBrush Press(FLinearColor(0, 1, 1, 0.5f));
	
	DefultButtonStyle.SetNormal(Normal);
	DefultButtonStyle.SetHovered(Hover);
	DefultButtonStyle.SetPressed(Press);
	ButtonStyle = DefultButtonStyle;

	FSlateBrush DefaultInnerBackground;
	DefaultInnerBackground.TintColor = FLinearColor(0, 0, 0);
	InnerBackground = DefaultInnerBackground;

	FSlateFontInfo FontInfo(InArgs._FontBase, 22);

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(500.f)
				.HeightOverride(200.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SImage)
						.Image(&InnerBackground)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(FMargin(20.f, 0))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.MaxHeight(150.f)
						.Padding(FMargin(0, 20.f))
						[
							SNew(STextBlock)
							.Text(InArgs._Content)
							.Font(FontInfo)
						]
						+ SVerticalBox::Slot()
						.MaxHeight(50.f)
						[
							SNew(SButton)
							.ButtonStyle(&ButtonStyle)
							.OnClicked(this, &SDialog::OnClick)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Dialog_SureText", "确定"))
								.Justification(ETextJustify::Center)
								.LineHeightPercentage(1.f)
								.Font(FontInfo)
								.ColorAndOpacity(FSlateColor(FLinearColor(1, 1, 1)))
							]
						]
					]
				]

			]
		];
}

FReply SDialog::OnClick()
{
	FGenericPlatformMisc::RequestExit(false);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE