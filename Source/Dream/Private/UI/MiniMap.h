// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MiniMap.generated.h"


enum ESegmentPosition
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
UCLASS()
class UMiniMap : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=MiniMap)
	FSlateBrush WarningSegmentExternal;
	UPROPERTY(EditAnywhere, Category=MiniMap)
	FSlateBrush WarningSegmentMiddle;
	UPROPERTY(EditAnywhere, Category=MiniMap)
	FSlateBrush WarningSegmentCenter;

	/**
	 * 距离目标多远时显示最外部的预警提示， 当 HighLightExternalThreshold < (玩家与目标的距离 / 小地图的检测距离) 显示
	 */
	UPROPERTY(EditAnywhere, Category=MiniMap, meta = (ClampMin = 0, ClampMax = 1))
	float HighLightExternalThreshold;

	/**
	 * 当 HighLightCenterThreshold > (玩家与目标的距离 / 小地图的检测距离) 高亮小地图中心的预警提示
	 */
	UPROPERTY(EditAnywhere, Category=MiniMap, meta = (ClampMin = 0, ClampMax = 1))
	float HighLightCenterThreshold;

	/**
	* 当 SpriteOverflowThreshold == (玩家与目标的距离 / 小地图的检测距离) 意味着绘制sprite已经溢出小地图, 这表示将会把绘制的sprite限制在一定范围之内
	*/
	UPROPERTY(EditAnywhere, Category=MiniMap, meta = (ClampMin = 0, ClampMax = 1))
	float SpriteOverflowThreshold;
	

protected:

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	
};
