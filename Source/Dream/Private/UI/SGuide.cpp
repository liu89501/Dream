// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGuide.h"
#include "SlateOptMacros.h"
#include "Kismet/GameplayStatics.h"

void SGuide::Construct(const FArguments& InArgs)
{
	Elements = InArgs._Elements;
	PlayerOwner = InArgs._PlayerOwner;
	Padding = InArgs._Padding;
	
	//const FGuideStyle& GuideStyle = FDreamStyle::Get().GetWidgetStyle<FGuideStyle>("GuideStyle");
}

int32 SGuide::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	APlayerController* Controller = PlayerOwner.Get();

	if (Elements.IsSet() && Controller)
	{
		const TArray<FGuideElement>& GuideElements = Elements.Get();

		for (const FGuideElement& Element : GuideElements)
		{
			const FSlateBrush* const GuideBrush = Element.Brush;

			if (GuideBrush == nullptr)
			{
				continue;
			}

			FVector2D ScreenPos;
			UGameplayStatics::ProjectWorldToScreen(Controller, Element.Location, ScreenPos);

			// 让轴点位于自身的中心
			ScreenPos -= GuideBrush->ImageSize * 0.5f;

			// 将图标钳制于屏幕中
			FVector2D MaxPos = AllottedGeometry.GetAbsoluteSize() - Padding - GuideBrush->ImageSize;
			ScreenPos.X = FMath::Clamp(ScreenPos.X, Padding.X, MaxPos.X);
			ScreenPos.Y = FMath::Clamp(ScreenPos.Y, Padding.Y, MaxPos.Y);

			//UE_LOG(LogTemp, Error, TEXT("ScreenPos: %s"), *ScreenPos.ToString());

			FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(
				GuideBrush->ImageSize, FSlateLayoutTransform(ScreenPos), FSlateRenderTransform());

			FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, GuideBrush);
		}
	}
	
	return LayerId;
}
