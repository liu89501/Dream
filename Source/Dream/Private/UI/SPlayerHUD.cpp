// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerHUD.h"
#include "SImage.h"
#include "SCanvas.h"
#include "SBoxPanel.h"
#include "Engine.h"
#include "SBox.h"
#include "SOverlay.h"
#include "STextBlock.h"
#include "SlateOptMacros.h"
#include "Dream.h"
#include "Engine/LocalPlayer.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPlayerHUD::Construct(const FArguments& InArgs)
{
	CrosshairBrush = InArgs._CrosshairBrush;
	MagazineBrush = InArgs._MagazineBrush;

	//FSlateFontInfo HealthFont(InArgs._FontBase, 12);
	FSlateFontInfo AmmosFont(InArgs._FontBase, 32);
	FSlateFontInfo MagFont(InArgs._FontBase, 14);

	FText DefaultText = FText::FromString(TEXT("0"));

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(CrossHair, SBox)
				.WidthOverride(180.f)
				.HeightOverride(180.f)
				[
					SNew(SImage)
					.Image(&CrosshairBrush)
				]

			]
			+ SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Left)
			.Padding(40.f)
			[
				SNew(SBox)
				.WidthOverride(300.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4.f)
					.AutoHeight()
					.HAlign(HAlign_Fill)
					[
						SNew(SBox)
						.HeightOverride(10.f)
						[
							SNew(SOverlay)
							+SOverlay::Slot()
							[
								SNew(SImage)
                                .Image(InArgs._HealthBrush)
							]
						]
						/*SNew(SOverlay)
						+ SOverlay::Slot()
						.VAlign(VAlign_Bottom)
						[
							SNew(SBox)
							.HeightOverride(10.f)
							[
								SNew(SImage)
								.Image(InArgs._HealthBrush)
							]
						]*/
						/*+ SOverlay::Slot()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Top)
						[
							SAssignNew(HealthText, STextBlock)
							.Margin(FMargin(6, 0))
							.Text(DefaultText)
							.Font(HealthFont)
						]*/
					]
					+ SVerticalBox::Slot()
					.Padding(4.f, 1.f)
					.HAlign(HAlign_Left)
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(6.f)
						.WidthOverride(210.f)
						[
							SNew(SImage)
							.Image(InArgs._ShieldBrush)
						]
						/*SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBox)
							.HeightOverride(5.f)
							.WidthOverride(240.f)
							[
								SNew(SImage)
								.Image(InArgs._ShieldBrush)
							]
						]*/
						/*+ SOverlay::Slot()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Top)
						[
							SAssignNew(ShieldText, STextBlock)
							.Margin(FMargin(6, 0))
							.Text(DefaultText)
							.Font(HealthFont)
						]*/
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(40.f)
			[
				SNew(SBox)
				.WidthOverride(300.f)
				[

					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Bottom)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Bottom)
						.AutoWidth()
						[
							SAssignNew(AmmoText, STextBlock)
							.Text(DefaultText)
							.Font(AmmosFont)
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Bottom)
						.Padding(FMargin(0, 0, 0, 4.f))
						.AutoWidth()
						[
							SNew(STextBlock)
							.Font(MagFont)
							.Text(FText::FromString(TEXT("/")))
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Bottom)
						.Padding(FMargin(0, 0, 0, 4.f))
						.AutoWidth()
						[
							SAssignNew(MagText, STextBlock)
							.Font(MagFont)
							.Text(DefaultText)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(10.f)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(&MagazineBrush)
						]
					]
				]
			]
		];
}

void SPlayerHUD::SetCrosshairBrush(const FSlateBrush& NewCrosshairBrush)
{
	CrosshairBrush = NewCrosshairBrush;
	SetCrosshairSize(NewCrosshairBrush.ImageSize);
}

void SPlayerHUD::SetMagazineBrush(const FSlateBrush& NewMagazineBrush)
{
	MagazineBrush = NewMagazineBrush;
}

void SPlayerHUD::SetHealthText(const FString& NewHealth)
{
	if (HealthText.IsValid())
	{
		HealthText->SetText(FText::FromString(NewHealth));
	}
}

void SPlayerHUD::SetShieldText(const FString& NewShield)
{
	if (ShieldText.IsValid())
	{
		ShieldText->SetText(FText::FromString(NewShield));
	}
}

void SPlayerHUD::SetAmmoText(const FString& NewAmmos)
{
	if (AmmoText.IsValid())
	{
		AmmoText->SetText(FText::FromString(NewAmmos));
	}
}

void SPlayerHUD::SetMagText(const FString& NewMagazine)
{
	if (MagText.IsValid())
	{
		MagText->SetText(FText::FromString(NewMagazine));
	}
}

UMaterialInstanceDynamic* SPlayerHUD::GetMagazineMaterial()
{
	return Cast<UMaterialInstanceDynamic>(MagazineBrush.GetResourceObject());
}

UMaterialInstanceDynamic* SPlayerHUD::GetCrosshairMaterial()
{
	return Cast<UMaterialInstanceDynamic>(CrosshairBrush.GetResourceObject());
}

void SPlayerHUD::SetCrosshairSize(FVector2D Size)
{
	CrossHair->SetWidthOverride(Size.X);
	CrossHair->SetHeightOverride(Size.Y);
}

void SPlayerHUD::SetCrossHairVisibility(EVisibility NewVisibility)
{
	CrossHair->SetVisibility(NewVisibility);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
