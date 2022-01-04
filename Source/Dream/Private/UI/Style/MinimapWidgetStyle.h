// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "MinimapWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct FMinimapStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FMinimapStyle();
	virtual ~FMinimapStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FMinimapStyle& GetDefault();

	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush PointerIconBrush;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush BackgroundBrush;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush WarningOuterBrush;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush WarningMiddleBrush;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush WarningCenterBrush;
};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UMinimapWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FMinimapStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return &WidgetStyle;
	}
};
