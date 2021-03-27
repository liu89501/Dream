// Fill out your copyright notice in the Description page of Project Settings.


#include "SSessionMenu.h"
#include "SlateOptMacros.h"
#include "SOverlay.h"
#include "STextBlock.h"
#include "SImage.h"
#include "SListView.h"
#include "STableRow.h"
#include "STableViewBase.h"
#include "SBorder.h"
#include "SBox.h"
#include "SBoxPanel.h"
#include "SBackgroundBlur.h"
#include "Engine.h"
#include "SButton.h"

#define LOCTEXT_NAMESPACE "FPS.GameMenu"

void SSessionMenu::Construct(const FArguments& InArgs)
{
	MenuFont = InArgs._MenuFont;
	MenuBorderBrush = InArgs._MenuItemBorderBrush;

	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBox)
			[
				SNew(SImage)
				.ColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, .5f))
			]
		]
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(8)
			[
				SNew(SBox)
				.WidthOverride(600.f)
				[
					SNew(SBorder)
					.BorderImage(&MenuBorderBrush)
					[
						SNew(STextBlock)
						.Margin(8)
						.Justification(ETextJustify::Center)
						.Font(FSlateFontInfo(MenuFont, 24))
						.Text(LOCTEXT("MainMenuItem_Search_Session", "搜索会话"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(8)
			[
				SNew(SBorder)
				.BorderImage(&MenuBorderBrush)
				.OnMouseButtonDown(this, &SSessionMenu::OnBackItemClick)
				[
					SNew(STextBlock)
					.Margin(8)
					.Justification(ETextJustify::Center)
					.Font(FSlateFontInfo(MenuFont, 24))
					.Text(LOCTEXT("MainMenuItem_Search_Back", "返回"))
				]
			]
			+ SVerticalBox::Slot()
			.Padding(8)
			[
				SNew(SBorder)
				.BorderImage(&MenuBorderBrush)
				.OnMouseButtonDown(this, &SSessionMenu::OnEndItemClick)
				[
					SNew(STextBlock)
					.Margin(8)
					.Justification(ETextJustify::Center)
					.Font(FSlateFontInfo(MenuFont, 24))
					.Text(LOCTEXT("MainMenuItem_Search_End", "退出游戏"))
				]
			]
		]
	];
}

FReply SSessionMenu::OnBackItemClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnBack.ExecuteIfBound();

	return FReply::Handled();
}

FReply SSessionMenu::OnEndItemClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnEnd.ExecuteIfBound();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE