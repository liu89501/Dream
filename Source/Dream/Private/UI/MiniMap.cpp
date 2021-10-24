// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniMap.h"
#include "DCharacterPlayer.h"

int32 UMiniMap::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                            FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                            bool bParentEnabled) const
{
    Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    const FLocalPlayerContext& LocalPlayerContext = GetPlayerContext();
    if (!LocalPlayerContext.IsValid())
    {
        return LayerId;
    }

    ADCharacterPlayer* PlayerPawn = Cast<ADCharacterPlayer>(LocalPlayerContext.GetPawn());
    if (PlayerPawn == nullptr || PlayerPawn->IsPendingKill())
    {
        return LayerId;
    }

    TArray<FMiniMapData> MiniMapData;
    if (!PlayerPawn->GetMiniMapTips(MiniMapData))
    {
        return LayerId;
    }

    TMap<ESegmentPosition, uint16> SegmentCount;

    for (const FMiniMapData& SpriteData : MiniMapData)
    {
        if (SpriteData.DrawType != EMiniMapDrawType::Warning)
        {
            float DistancePercentage = FMath::Min(SpriteData.DistancePercentage, SpriteOverflowThreshold);
            const FSlateBrush* SpriteBrush = DistancePercentage == SpriteOverflowThreshold
                                                 ? SpriteData.OverflowSpriteBrush
                                                 : SpriteData.SpriteBrush;

            if (SpriteBrush && SpriteBrush->HasUObject())
            {
                const FVector2D& GeometryLocalSize = AllottedGeometry.GetLocalSize();
                FVector2D CenterPos = (GeometryLocalSize * 0.5f - SpriteBrush->ImageSize * 0.5f);
                FVector2D Position = (SpriteData.PosNormalize * -1.f) * (DistancePercentage * GeometryLocalSize.X * 0.5f);
                FVector2D FinalPos(CenterPos.Y - Position.Y, CenterPos.X + Position.X);

                FPaintGeometry Geometry = AllottedGeometry.ToPaintGeometry(SpriteBrush->ImageSize,
                                                                           FSlateLayoutTransform(FinalPos),
                                                                           FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(SpriteData.Yaw))));

                FSlateDrawElement::MakeBox(
                    OutDrawElements,
                    LayerId,
                    Geometry,
                    SpriteBrush,
                    ESlateDrawEffect::None,
                    SpriteBrush->TintColor.GetSpecifiedColor());
            }
        }
        else
        {
            ESegmentPosition SP;
            if (SpriteData.DistancePercentage < HighLightCenterThreshold)
            {
                SP = Center;
            }
            else
            {
                bool bOutOfScanRange = SpriteData.DistancePercentage > HighLightExternalThreshold;
                float YawAbs = FMath::Abs(SpriteData.Yaw);

                if (YawAbs > 157.5f)
                {
                    SP = bOutOfScanRange ? E180 : M180;
                }
                else if (YawAbs > 112.5f)
                {
                    SP = bOutOfScanRange ? E135 : M135;
                    if (SpriteData.Yaw > 0)
                    {
                        SP = bOutOfScanRange ? E135 : M135;
                    }
                    else
                    {
                        SP = bOutOfScanRange ? EN135 : MN135;
                    }
                }
                else if (YawAbs > 67.5f)
                {
                    if (SpriteData.Yaw > 0)
                    {
                        SP = bOutOfScanRange ? E90 : M90;
                    }
                    else
                    {
                        SP = bOutOfScanRange ? EN90 : MN90;
                    }
                }
                else if (YawAbs > 22.5f)
                {
                    if (SpriteData.Yaw > 0)
                    {
                        SP = bOutOfScanRange ? E45 : M45;
                    }
                    else
                    {
                        SP = bOutOfScanRange ? EN45 : MN45;
                    }
                }
                else
                {
                    SP = bOutOfScanRange ? E0 : M0;
                }
            }

            uint16& Count = SegmentCount.FindOrAdd(SP);
            Count++;
        }
    }

    if (SegmentCount.Num() == 0)
    {
        return LayerId;
    }

    for (TPair<ESegmentPosition, uint16> SegmentPair : SegmentCount)
    {
        const FSlateBrush* DrawBrush = SegmentPair.Key == Center
                                           ? &WarningSegmentCenter
                                           : SegmentPair.Key % 2 == 0
                                           ? &WarningSegmentMiddle
                                           : &WarningSegmentExternal;

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

        FLinearColor LinearColor = WarningSegmentExternal.TintColor.GetSpecifiedColor();
        LinearColor.A = FMath::Min(LinearColor.A + SegmentPair.Value * 0.2f, .9f);
        LinearColor.G = FMath::Max(LinearColor.G - SegmentPair.Value * 0.02f, 0.f);
        LinearColor.B = FMath::Max(LinearColor.B - SegmentPair.Value * 0.02f, 0.f);

        FSlateLayoutTransform LLayoutTransform(FVector2D::ZeroVector);
        FSlateRenderTransform LRenderTransform(FQuat2D(FMath::DegreesToRadians(Angle)));

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(AllottedGeometry.GetLocalSize(), LLayoutTransform, LRenderTransform),
            DrawBrush,
            ESlateDrawEffect::None,
            LinearColor);
    }

    return LayerId;
}
