// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 *	游戏内UI  武器准心，血条等
 */
class SPlayerHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPlayerHUD)
	{}

	SLATE_ARGUMENT(FSlateBrush, CrosshairBrush);

	SLATE_ARGUMENT(FSlateBrush, MagazineBrush);

	SLATE_ARGUMENT(const FSlateBrush*, HealthBrush);

	SLATE_ARGUMENT(const FSlateBrush*, ShieldBrush);

	SLATE_ARGUMENT(UFont*, FontBase);

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void SetCrosshairBrush(const FSlateBrush& NewCrosshairBrush);

	void SetMagazineBrush(const FSlateBrush& NewMagazineBrush);

	FORCEINLINE void SetCrosshairBrushColor(FLinearColor NewColor)
	{
		CrosshairBrush.TintColor = NewColor;
	}

	void SetHealthText(const FString& NewHealth);

	void SetShieldText(const FString& NewShield);

	void SetAmmoText(const FString& NewAmmos);

	void SetMagText(const FString& NewMagazine);

	void SetCrosshairSize(FVector2D Size);

	void SetCrossHairVisibility(EVisibility NewVisibility);

	UMaterialInstanceDynamic* GetMagazineMaterial();
	UMaterialInstanceDynamic* GetCrosshairMaterial();

private:

	TSharedPtr<class STextBlock> HealthText;
	TSharedPtr<class STextBlock> ShieldText;
	TSharedPtr<class STextBlock> AmmoText;
	TSharedPtr<class STextBlock> MagText;

	TSharedPtr<class SBox> CrossHair;

	FSlateBrush CrosshairBrush;

	FSlateBrush MagazineBrush;
};
