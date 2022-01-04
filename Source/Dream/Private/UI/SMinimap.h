// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MinimapScanComponent.h"
#include "Style/MinimapWidgetStyle.h"
#include "Widgets/SCompoundWidget.h"

struct FMiniMapData;

enum EPosition
{
	M0,
    E0,
    M45,
    E45,
    MN45,
    EN45,
    M90,
    E90,
    MN90,
    EN90,
    M135,
    E135,
    MN135,
    EN135,
    M180,
    E180,
    Center
};

/**
 * 
 */
class SMinimap : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMinimap)
		: _PartCenter(0.1f),
		  _PartOuter(0.9f)
	{}


	/**
	 * 传入的Distance小于这个值表示为小地图的中心
	 */
	SLATE_ARGUMENT(float, PartCenter);

	/**
	 * 传入的Distance小于这个值 大于PartCenter 表示为小地图外部
	 */
	SLATE_ARGUMENT(float, PartOuter);

	SLATE_ATTRIBUTE(FMinimapDataIterator, DrawItemsIterator);

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override;


protected:

	float PartCenter;
	
	float PartOuter;
	
	FMinimapStyle MinimapStyle;
	
	TAttribute<FMinimapDataIterator> Iterator;
};
