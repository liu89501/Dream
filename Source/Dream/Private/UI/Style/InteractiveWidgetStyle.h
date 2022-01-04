// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "InteractiveWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct FInteractiveStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FInteractiveStyle();
	virtual ~FInteractiveStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FInteractiveStyle& GetDefault();

	UPROPERTY(Category=Appearance, EditAnywhere)
	FSlateBrush KeyBackgroundBrush;
	
	UPROPERTY(Category=Appearance, EditAnywhere)
	FSlateBrush PressedProgressBrush;
	
	UPROPERTY(Category=Appearance, EditAnywhere)
	FTextBlockStyle KeyFontStyle;
	
	UPROPERTY(Category=Appearance, EditAnywhere)
	FTextBlockStyle HintFontStyle;

	UPROPERTY(Category=Appearance, EditAnywhere)
	USoundCue* InteractiveSoundCue;
};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UInteractiveWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FInteractiveStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return &WidgetStyle;
	}
};
