// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "DialogSlateWidgetStyle.generated.h"

USTRUCT(BlueprintType)
struct FDialogInformation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText DialogTitle;

	UPROPERTY(EditAnywhere)
	FSlateBrush DialogIcon;
};

UENUM()
enum class EDialogType : uint8
{
	INFO,
    WARNING,
    ERROR
};

/**
 * 
 */
USTRUCT()
struct FDialogSlateStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FDialogSlateStyle();
	virtual ~FDialogSlateStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FDialogSlateStyle& GetDefault();

	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush BackgroundBrush;

	UPROPERTY(EditAnywhere, Category=Appearance)
	FTextBlockStyle TitleTextStyle;
	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FTextBlockStyle ContentTextStyle;

	UPROPERTY(EditAnywhere, Category=Appearance)
	TMap<EDialogType, FDialogInformation> Icons;
};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UDialogSlateWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FDialogSlateStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &WidgetStyle );
	}
};
