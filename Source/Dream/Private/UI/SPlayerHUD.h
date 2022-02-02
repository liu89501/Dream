// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SlateUtils.h"
#include "SInteractive.h"
#include "MinimapScanComponent.h"
#include "Components/TimelineComponent.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class ADCharacterPlayer;
class SProgressBar;

/**
 *	武器准心, 血条, 小地图等
 */
class SPlayerHUD : public SCompoundWidget, FGCObject
{
public:
	SLATE_BEGIN_ARGS(SPlayerHUD)
		: _InOwnerPlayer(nullptr),
		  _HitMarkDisplayTime(1.f),
		  _StateGridMaxColumn(10)
	{}

		SLATE_ARGUMENT(ADCharacterPlayer*, InOwnerPlayer);
	
		SLATE_ARGUMENT(float, HitMarkDisplayTime);

		SLATE_ARGUMENT(int32, StateGridMaxColumn);
	
		SLATE_ATTRIBUTE(FMinimapDataIterator, MinimapDataIterator);

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** 减少生命值 */
	void ReduceHealth(float Percentage);

	/** 恢复生命值 */
	void RecoveryHealth(float Percentage);

	void SetCrosshairBrush(const FSlateBrush& NewCrosshairBrush) const;

	void SetMagazineInformation(int32 InAmmo, int32 InMagazine, float AmmoPercentage) const;

	void SetCrossHairVisibility(EVisibility NewVisibility) const;

	void ShowHitEnemyMark(bool bEnemyDeath);

	void ShowHurtMark(float Direction);

	void AddStateIcon(const FGameplayTag& Tag, UTexture2D* Icon, const int32 StackCount, float Duration);

	void RefreshStateIcon(const FGameplayTag& Tag, const int32 StackCount, float NewDuration);

	void RemoveStateIcon(const FGameplayTag& Tag);

	void ActivateInteractiveButton(float InteractiveTime, const FText& InteractiveText, FOnInteractiveCompleted Delegate) const;

	/** 仅仅只是显示交互按钮，并不绑定输入事件 */
	void DisplayInteractiveButton(const FText& InteractiveText);

	void DeactivateInteractiveButton() const;

public:

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

private:

	void HiddenHitMark();

	void OnHurtMarkOpacityUpdate(float Value) const;

private:

	TSharedPtr<STextBlock> AmmoText;
	TSharedPtr<STextBlock> MagText;

	TSharedPtr<SImage> Crosshair;
	
	TSharedPtr<SImage> HitMark;
	FLinearAnimation HitMarkDisplayTime;

	int32 StateGridMaxColumn;
	TSharedPtr<class SUniformGridPanel> StatePanel;

	TSharedPtr<SImage> HurtMark;
	FTimeline HurtAnim;
	FSlateBrush HurtBrush;
	UMaterialInstanceDynamic* HurtDynamic;

	FLinearAnimation HealthBackgroundAnim;
	FLinearAnimation HealthRecoveryAnim;
	TSharedPtr<SProgressBar> HealthBar;
	TSharedPtr<SProgressBar> HealthBackgroundBar;
	float HealthBarPercent;
	
	TSharedPtr<SProgressBar> Magazine;

	TWeakObjectPtr<ADCharacterPlayer> OwnerPlayer;

	TSharedPtr<SInteractive> InteractiveButton;
	
	TMap<FGameplayTag, TSharedPtr<class SStateIcon>> StateIcons;
};



