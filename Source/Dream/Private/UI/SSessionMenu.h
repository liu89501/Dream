// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateColorBrush.h"
#include "SlateBorderBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

/**
 *
 */
class SSessionMenu : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SSessionMenu)
		: _MenuItemBorderBrush(FSlateColorBrush(FLinearColor(0, 0, 0, 0.7f)))
	{}

	SLATE_ARGUMENT(UObject*, MenuFont);

	SLATE_ARGUMENT(FSlateBrush, MenuItemBorderBrush);

	SLATE_ARGUMENT(class AFirstPersonPlayerShooter*, PlayerCtrl);

	SLATE_END_ARGS()

	DECLARE_DELEGATE(FMenuBackItemClick)

	DECLARE_DELEGATE(FMenuEndItemClick)

	FMenuBackItemClick OnBack;

	FMenuEndItemClick OnEnd;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual FReply OnBackItemClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	virtual FReply OnEndItemClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

private:

	FSlateBrush MenuBorderBrush;

	UObject* MenuFont;
};
