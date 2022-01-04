// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SMinimap.h"
#include "SImage.h"
#include "SlateOptMacros.h"
#include "Kismet/KismetMathLibrary.h"
#include "Style/DreamStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMinimap::Construct(const FArguments& InArgs)
{
	PartCenter = InArgs._PartCenter;
	PartOuter = InArgs._PartOuter;
	Iterator = InArgs._DrawItemsIterator;

	MinimapStyle = FDreamStyle::Get().GetWidgetStyle<FMinimapStyle>("MinimapStyle");

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&MinimapStyle.BackgroundBrush)
		]
		+ SOverlay::Slot()
		  .HAlign(HAlign_Center)
		  .VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(&MinimapStyle.PointerIconBrush)
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SMinimap::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                        FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                        bool bParentEnabled) const
{
	SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                         bParentEnabled);


	if (!Iterator.IsSet())
	{
		return LayerId;
	}

	LayerId++;

	TMap<EPosition, uint8> SegmentCount;

	const FMinimapDataIterator& MDIterator = Iterator.Get();

	for (; MDIterator; ++MDIterator)
	{
		TSharedPtr<FMinimapDataBase> MinimapData = MDIterator.Get();

		if (!MinimapData.IsValid())
		{
			continue;
		}

		if (MinimapData->DataType == EMinimapDataType::Warning)
		{
			EPosition Pos;
			if (MinimapData->Distance < PartCenter)
			{
				Pos = Center;
			}
			else
			{
				bool bOutOfScanRange = MinimapData->Distance > PartOuter;
				float YawAbs = FMath::Abs(MinimapData->Direction);

				if (YawAbs > 157.5f)
				{
					Pos = bOutOfScanRange ? E180 : M180;
				}
				else if (YawAbs > 112.5f)
				{
					Pos = bOutOfScanRange ? E135 : M135;
					if (MinimapData->Direction > 0)
					{
						Pos = bOutOfScanRange ? E135 : M135;
					}
					else
					{
						Pos = bOutOfScanRange ? EN135 : MN135;
					}
				}
				else if (YawAbs > 67.5f)
				{
					if (MinimapData->Direction > 0)
					{
						Pos = bOutOfScanRange ? E90 : M90;
					}
					else
					{
						Pos = bOutOfScanRange ? EN90 : MN90;
					}
				}
				else if (YawAbs > 22.5f)
				{
					if (MinimapData->Direction > 0)
					{
						Pos = bOutOfScanRange ? E45 : M45;
					}
					else
					{
						Pos = bOutOfScanRange ? EN45 : MN45;
					}
				}
				else
				{
					Pos = bOutOfScanRange ? E0 : M0;
				}
			}

			uint8& Count = SegmentCount.FindOrAdd(Pos);
			Count++;
		}
		else if (MinimapData->DataType == EMinimapDataType::Icon)
		{
			const FMiniMapIcon* MiniMapIcon = static_cast<FMiniMapIcon*>(MinimapData.Get());

			const FVector2D& LocalSize = AllottedGeometry.GetLocalSize();
			
			FVector2D CenterPos = LocalSize * 0.5f - MiniMapIcon->Icon->ImageSize * 0.5f;
			FVector2D Position = MiniMapIcon->Position * -1.f * (MiniMapIcon->Distance * LocalSize.X * 0.5f);
			FVector2D FinalPos(CenterPos.Y - Position.Y, CenterPos.X + Position.X);

			FPaintGeometry Geometry = AllottedGeometry.ToPaintGeometry(
				MiniMapIcon->Icon->ImageSize,
				FSlateLayoutTransform(FinalPos),
				FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(MinimapData->Direction))));

			FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry,
			                           MiniMapIcon->Icon, ESlateDrawEffect::None,
			                           MiniMapIcon->Icon->TintColor.GetSpecifiedColor());
		}
	}

	if (SegmentCount.Num() == 0)
	{
		return LayerId;
	}

	for (TPair<EPosition, uint8> SegmentPair : SegmentCount)
	{
		const FSlateBrush* DrawBrush = SegmentPair.Key == Center
			                               ? &MinimapStyle.WarningCenterBrush
			                               : SegmentPair.Key % 2 == 0
			                               ? &MinimapStyle.WarningMiddleBrush
			                               : &MinimapStyle.WarningOuterBrush;

		float Angle;

		switch (SegmentPair.Key)
		{
		case E0:
		case M0:
		case Center:
			Angle = 0.f;
			break;
		case E180:
		case M180:
			Angle = 180.f;
			break;
		case E90:
		case M90:
			Angle = 90.f;
			break;
		case MN90:
		case EN90:
			Angle = -90.f;
			break;
		case M45:
		case E45:
			Angle = 45.f;
			break;
		case MN45:
		case EN45:
			Angle = -45.f;
			break;
		case M135:
		case E135:
			Angle = 135.f;
			break;
		case MN135:
		case EN135:
			Angle = -135.f;
			break;
		}

		FLinearColor LinearColor = MinimapStyle.WarningOuterBrush.TintColor.GetSpecifiedColor();
		LinearColor.A = UKismetMathLibrary::MapRangeClamped(SegmentPair.Value, 1, 8, 0.2f, 0.6f);

		FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(
			AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(), FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(Angle))));

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			PaintGeometry,
			DrawBrush,
			ESlateDrawEffect::None,
			LinearColor);
	}

	return LayerId;
}
