// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "PlayerHUDStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct FPlayerHUDWidgetStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FPlayerHUDWidgetStyle();
	virtual ~FPlayerHUDWidgetStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FPlayerHUDWidgetStyle& GetDefault();

	UPROPERTY(EditAnywhere, Category=Appearance)
	FProgressBarStyle HealthBarStyle;

	UPROPERTY(EditAnywhere, Category=Appearance)
	FProgressBarStyle HealthBarBackgroundStyle;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FProgressBarStyle MagazineStyle;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush HitMarkBrush;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush HurtBrush;

	UPROPERTY(EditAnywhere, Category=Appearance)
	UCurveFloat* HurtAnimCurve;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FTextBlockStyle MagazineTextStyle;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FTextBlockStyle AmmoTextStyle;
};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UPlayerHUDStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FPlayerHUDWidgetStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return &WidgetStyle;
	}
};
