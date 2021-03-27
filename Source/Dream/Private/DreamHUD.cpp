// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DreamHUD.h"

ADreamHUD::ADreamHUD()
{
	HitRemindDuration = 0.5f;

	HitRemindColor = FLinearColor::White;
}

void ADreamHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ADreamHUD::DrawHUD()
{
	Super::DrawHUD();
	//DrawStateBar();
	//DrawMagazine();
	/*DrawMinimalMap();
	DrawCrosshair();
	DrawHitRemind();*/
}

//void ADreamHUD::DrawStateBar()
//{
//	if (OwningShooter->IsValidLowLevel())
//	{
//		FVector2D PawnIconPos(30.f, Canvas->ClipY - PawnIcon.Size.Y - 20.f);
//
//		FCanvasTileItem IconItem(PawnIconPos, PawnIcon.Texture->Resource, PawnIcon.Size, FLinearColor::White);
//		IconItem.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;
//
//		Canvas->DrawItem(IconItem);
//
//		AFirstPersonPlayerShooter* ShooterCDO = OwningShooter->GetClass()->GetDefaultObject<AFirstPersonPlayerShooter>();
//
//		FVector2D ShieldMargin(PawnIcon.Size.X + 40.f, 20.f + Shield.Size.Y);
//		FVector2D HealthMargin(PawnIcon.Size.X + 40.f, 30.f + Shield.Size.Y + Health.Size.Y);
//
//		FVector2D ShieldBarPosition(ShieldMargin.X, Canvas->ClipY - ShieldMargin.Y);
//		FVector2D HealthBarPosition(HealthMargin.X, Canvas->ClipY - HealthMargin.Y);
//
//		float HealthPrecent = OwningShooter->Health / ShooterCDO->Health;
//		float ShieldPrecent = OwningShooter->Shield / ShooterCDO->Shield;
//
//		//NetDebugMsg(TEXT("%f, %f"), HealthPrecent, ShieldPrecent);
//
//		if (!FMath::IsNearlyEqual(HealthPrecent, PrevHealthPrecent, .001f))
//		{
//			PrevHealthPrecent = HealthPrecent;
//			HealthBarDynamic->SetScalarParameterValue(PrecentName, HealthPrecent);
//		}
//
//		if (!FMath::IsNearlyEqual(ShieldPrecent, PrevShieldPrecent, .001f))
//		{
//			PrevShieldPrecent = ShieldPrecent;
//			ShieldBarDynamic->SetScalarParameterValue(PrecentName, ShieldPrecent);
//		}
//
//		FCanvasTileItem HealthBar(HealthBarPosition, HealthBarDynamic->GetRenderProxy(), Health.Size);
//		FCanvasTileItem ShieldBar(ShieldBarPosition, ShieldBarDynamic->GetRenderProxy(), Shield.Size);
//
//		Canvas->DrawItem(ShieldBar);
//		Canvas->DrawItem(HealthBar);
//	}
//}
//
//void ADreamHUD::DrawMagazine()
//{
//	if (OwningShooter->IsValidLowLevel())
//	{
//		if (AShootWeapon* Weapon = OwningShooter->GetActiveWeapon())
//		{
//			FText AmmosText = FText::AsNumber(Weapon->Ammos);
//			FText MagazineText = FText::AsNumber(Weapon->MagazineAmmos);
//
//			int32 AmmosWidth;
//			int32 AmmosHeight;
//			AmmoFont->GetStringHeightAndWidth(*AmmosText.ToString(), AmmosHeight, AmmosWidth);
//
//			int32 MagazineWidth;
//			int32 MagazineHeight;
//			TotalAmmoFont->GetStringHeightAndWidth(*MagazineText.ToString(), MagazineHeight, MagazineWidth);
//
//			FVector2D AmmosTextMargin(80.f, 50.f);
//			FVector2D MagazineTextMargin(50.f, 50.f);
//
//			float AmmosX = Canvas->ClipX - AmmosTextMargin.X - AmmosWidth;
//			float AmmosY = Canvas->ClipY - AmmosTextMargin.Y;
//
//			float MagazineX = Canvas->ClipX - MagazineTextMargin.X - MagazineWidth;
//			float MagazineY = Canvas->ClipY - MagazineTextMargin.Y + AmmosHeight - MagazineHeight;
//
//			FCanvasTextItem AmmosItem(FVector2D(AmmosX, AmmosY), AmmosText, AmmoFont, FLinearColor::White);
//			FCanvasTextItem MagazineItem(FVector2D(MagazineX, MagazineY), MagazineText, TotalAmmoFont, FLinearColor::Gray);
//			Canvas->DrawItem(AmmosItem);
//			Canvas->DrawItem(MagazineItem);
//		}
//	}
//}

//void ADreamHUD::DrawCrosshair()
//{
//	if (OwningShooter->IsValidLowLevel())
//	{
//		AShootWeapon* Weapon = OwningShooter->GetActiveWeapon();
//
//		if (Weapon && !Weapon->IsHideCrosshiar())
//		{
//			if (UMaterialInstanceDynamic* Dynamic = Weapon->GetCrosshairDynamicMaterial())
//			{
//				FVector2D CrosshairSize(200.f, 200.f);
//				FVector2D CrosshairPos(Canvas->ClipX * .5f - CrosshairSize.X * .5f, Canvas->ClipY * .5f - CrosshairSize.Y * .5f);
//				FCanvasTileItem CrossItem(CrosshairPos, Dynamic->GetRenderProxy(), CrosshairSize);
//				Canvas->DrawItem(CrossItem);
//			}
//		}
//	}
//}

//void ADreamHUD::DrawMinimalMap()
//{
//	if (MinimalMap)
//	{
//		float SizeXY = Canvas->ClipY * .2f;
//
//		FVector2D MapSize(SizeXY, SizeXY);
//		FVector2D Margin(20.f + MapSize.X, 20.f);
//		FVector2D Position(Canvas->ClipX - Margin.X, Margin.Y);
//
//		FCanvasTileItem MapItem(Position, MinimalMap->GetRenderProxy(), MapSize);
//		Canvas->DrawItem(MapItem);
//	}
//}
//
//void ADreamHUD::DrawHitRemind()
//{
//	if (bShowHitRemindTips)
//	{
//		HitRemindDuration = FMath::Max(HitRemindDuration - RenderDelta, 0.f);
//
//		FVector2D Size(30.f, 30.f);
//		FVector2D Position(Canvas->ClipX * .5f - Size.X * .5f, Canvas->ClipY * .5f - Size.Y * .5f);
//		FCanvasTileItem HitTipsTile(Position, HitRemindTexture->Resource, Size, HitRemindColor);
//		HitTipsTile.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;
//		Canvas->DrawItem(HitTipsTile);
//
//		if (HitRemindDuration == 0.f)
//		{
//			bShowHitRemindTips = false;
//			HitRemindDuration = Cast<ADreamHUD>(GetClass()->ClassDefaultObject)->HitRemindDuration;
//		}
//	}
//}

//void ADreamHUD::ShowHitRemind(FLinearColor NewRemindColor)
//{
//	bShowHitRemindTips = true;
//
//	HitRemindColor = NewRemindColor;
//}
