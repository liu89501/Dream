// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerHUD.h"
#include "SMinimap.h"
#include "DCharacterPlayer.h"
#include "DMPlayerController.h"
#include "SImage.h"
#include "SCanvas.h"
#include "SBoxPanel.h"
#include "Engine.h"
#include "SOverlay.h"
#include "STextBlock.h"
#include "SlateOptMacros.h"
#include "SStateIcon.h"
#include "Style/DreamStyle.h"
#include "Style/PlayerHUDStyle.h"

#define LOCTEXT_NAMESPACE "PlayerHUD"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPlayerHUD::Construct(const FArguments& InArgs)
{
	const ISlateStyle& SlateStyle = FDreamStyle::Get();
	const FPlayerHUDWidgetStyle& HUDStyle = SlateStyle.GetWidgetStyle<FPlayerHUDWidgetStyle>("PlayerHUDStyle");
	const FSlateBrush* BlackBackgroundBrush = SlateStyle.GetBrush("Dream.BlackTranslucentBackground");

	OwnerPlayer = InArgs._InOwnerPlayer;
	StateGridMaxColumn = InArgs._StateGridMaxColumn;

	HitMarkDisplayTime = InArgs._HitMarkDisplayTime;

	HealthBarPercent = 1.f;

	// 受伤标记
	HurtBrush = HUDStyle.HurtBrush;
	HurtDynamic = SlateUtils::GetDynamicFromBrush(HurtBrush, InArgs._InOwnerPlayer);
	HurtAnim.AddInterpFloat(HUDStyle.HurtAnimCurve,
        FOnTimelineFloatStatic::CreateRaw(this, &SPlayerHUD::OnHurtMarkOpacityUpdate));

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(Crosshair, SImage)
                .Visibility(EVisibility::Collapsed)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(HitMark, SImage)
				.Image(&HUDStyle.HitMarkBrush)
                .Visibility(EVisibility::Collapsed)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
			[
				SAssignNew(HurtMark, SImage)
				.Image(&HurtBrush)
                .RenderOpacity(0)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(30.f)
			[
				SNew(SBox)
				.HeightOverride(260.f)
				.WidthOverride(260.f)
				[
					SNew(SMinimap)
					.DrawItemsIterator(InArgs._MinimapDataIterator)
				]
			]
			+ SOverlay::Slot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Bottom)
            .Padding(0, 0, 0, 80.f)
            [
	            SAssignNew(StatePanel, SUniformGridPanel)
	            .SlotPadding(FVector2D(1.f, 1.f))
	            .RenderTransform(FTransform2D(FQuat2D(FVector2D(-1, 0))))  // 将panel倒过来, 这么做的目的是为了让内容从下往上增长
	            .RenderTransformPivot(FVector2D(0.5f, 0.5f))
            ]
			+ SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			.Padding(40.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(6.f)
				.HAlign(HAlign_Fill)
				.AutoHeight()
				[
					SNew(SVerticalBox)
	                + SVerticalBox::Slot()
	                .VAlign(VAlign_Bottom)
	                .HAlign(HAlign_Right)
	                .Padding(FMargin(0, 0, 0, 4.f))
	                [
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Bottom)
						.AutoWidth()
						[
							SAssignNew(AmmoText, STextBlock)
	                        .TextStyle(&HUDStyle.AmmoTextStyle)
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Bottom)
						.AutoWidth()
						[
							SNew(STextBlock)
	                        .TextStyle(&HUDStyle.AmmoTextStyle)
	                        .Text(FText::FromString(TEXT("/")))
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Bottom)
						.AutoWidth()
						[
							SAssignNew(MagText, STextBlock)
	                        .TextStyle(&HUDStyle.MagazineTextStyle)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
		                .HeightOverride(8.f)
		                [
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Image(BlackBackgroundBrush)
							]
							+ SOverlay::Slot()
							.Padding(1)
							[
								SAssignNew(Magazine, SProgressBar)
								.Style(&HUDStyle.MagazineStyle)
								.BarFillType(EProgressBarFillType::RightToLeft)
							]
                		]
					]
				]
				+ SVerticalBox::Slot()
                .Padding(6.f)
                .AutoHeight()
                .HAlign(HAlign_Fill)
                [
                	SNew(SBox)
                	.HeightOverride(8.f)
                	.WidthOverride(240.f)
                	[
	                    SNew(SOverlay)
	                    + SOverlay::Slot()
	                    [
							SNew(SImage)
                            .Image(BlackBackgroundBrush)
	                    ]
	                    + SOverlay::Slot()
	                    .Padding(1)
	                    [
		                    SAssignNew(HealthBackgroundBar, SProgressBar)
                            .Style(&HUDStyle.HealthBarBackgroundStyle)
                            .BarFillType(EProgressBarFillType::RightToLeft)
	                    ]
	                    + SOverlay::Slot()
	                    .Padding(1)
	                    [
		                    SAssignNew(HealthBar, SProgressBar)
		                    .Style(&HUDStyle.HealthBarStyle)
		                    .Percent(HealthBarPercent)
		                    .BarFillType(EProgressBarFillType::RightToLeft)
	                    ]
                	]
                ]
			]
			+ SOverlay::Slot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Bottom)
            .Padding(0, 0, 0, 210.f)
            [
				SAssignNew(InteractiveButton, SInteractive)
                .InPlayerOwner(InArgs._InOwnerPlayer->GetPlayerController())
                .Visibility(EVisibility::Collapsed)
            ]
		];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SPlayerHUD::ReduceHealth(float Percentage)
{
	HealthBackgroundAnim = Percentage;
	HealthBackgroundAnim.Activate();

	HealthBarPercent = Percentage;
	HealthBar->SetPercent(Percentage);
}

void SPlayerHUD::RecoveryHealth(float Percentage)
{
	HealthRecoveryAnim.Set(HealthBarPercent, Percentage);
	HealthRecoveryAnim.Activate();
}

void SPlayerHUD::SetCrosshairBrush(const FSlateBrush& NewCrosshairBrush) const
{
	Crosshair->SetImage(&NewCrosshairBrush);
}

void SPlayerHUD::SetMagazineInformation(int32 InAmmo, int32 InMagazine, float AmmoPercentage) const
{
	if (MagText.IsValid())
	{
		MagText->SetText(FText::AsNumber(InMagazine));
	}

	if (AmmoText.IsValid())
	{
		AmmoText->SetText(FText::AsNumber(InAmmo));
	}

	Magazine->SetPercent(AmmoPercentage);
}

void SPlayerHUD::SetCrossHairVisibility(EVisibility NewVisibility) const
{
	Crosshair->SetVisibility(NewVisibility);
}

void SPlayerHUD::ShowHitEnemyMark(bool bEnemyDeath)
{
	HitMark->SetVisibility(EVisibility::SelfHitTestInvisible);
	HitMark->SetColorAndOpacity(bEnemyDeath ? FLinearColor::Red : FLinearColor::White);
	HitMarkDisplayTime.Reset();
	HitMarkDisplayTime.Activate();
}

void SPlayerHUD::HiddenHitMark()
{
	HitMark->SetVisibility(EVisibility::Collapsed);
	HitMarkDisplayTime.Deactivate();
}

void SPlayerHUD::ShowHurtMark(float Direction)
{
	HurtDynamic->SetScalarParameterValue("Dir", Direction);
	HurtAnim.PlayFromStart();
}

void SPlayerHUD::OnHurtMarkOpacityUpdate(float Value) const
{
	if (HurtMark.IsValid())
	{
		HurtMark->SetRenderOpacity(Value);
	}
}

void SPlayerHUD::AddStateIcon(const FGameplayTag& StateTag, UTexture2D* Icon, const int32 StackCount, float Duration)
{
	TSharedRef<SStateIcon> StateIcon = SNew(SStateIcon)
		.RenderTransform(FTransform2D(FQuat2D(FVector2D(-1, 0)))) // 将图标翻转180度, 倒过来
        .Duration(Duration)
        .IconResource(Icon)
        .StackCount(StackCount);

	int32 StateIconNum = StatePanel->GetChildren()->Num();

	int32 Row = StateIconNum / StateGridMaxColumn;
	int32 Column = StateIconNum % StateGridMaxColumn;

	StatePanel->AddSlot(Column, Row)
	[
		StateIcon
	];

	StateIcons.Add(StateTag, StateIcon);
}

void SPlayerHUD::RefreshStateIcon(const FGameplayTag& StateTag, const int32 StackCount, float NewDuration)
{
	if (TSharedPtr<SStateIcon>* StateIconPtr = StateIcons.Find(StateTag))
	{
		TSharedPtr<SStateIcon>& StateIcon = *StateIconPtr;

		if (StateIcon.IsValid())
		{
			StateIcon->RefreshState(StackCount, NewDuration);
		}
	}
}

void SPlayerHUD::RemoveStateIcon(const FGameplayTag& StateTag)
{
	TSharedPtr<SStateIcon> StateIcon;
	if (StateIcons.RemoveAndCopyValue(StateTag, StateIcon))
	{
		StatePanel->RemoveSlot(StateIcon.ToSharedRef());

		// 刷新buff图标的位置
		TPanelChildren<SUniformGridPanel::FSlot>* Children =
			static_cast<TPanelChildren<SUniformGridPanel::FSlot>*>(StatePanel->GetChildren());

		for (int32 Idx = 0; Idx < Children->Num(); Idx++)
		{
			SUniformGridPanel::FSlot& Slot = (*Children)[Idx];
			Slot.Column = Idx % StateGridMaxColumn;
			Slot.Row = Idx / StateGridMaxColumn;
		}
	}
}

void SPlayerHUD::ActivateInteractiveButton(float InteractiveTime, const FText& InteractiveText, FOnInteractiveCompleted Delegate) const
{
	InteractiveButton->SetInteractiveText(InteractiveText);
	InteractiveButton->SetInteractiveTime(InteractiveTime);
	InteractiveButton->SetOnCompleted(Delegate);
	InteractiveButton->SetVisibility(EVisibility::SelfHitTestInvisible);
	InteractiveButton->EnableInput();
}

void SPlayerHUD::DisplayInteractiveButton(const FText& InteractiveText)
{
	InteractiveButton->SetInteractiveText(InteractiveText);
	InteractiveButton->SetVisibility(EVisibility::SelfHitTestInvisible);
}

void SPlayerHUD::DeactivateInteractiveButton() const
{
	InteractiveButton->SetVisibility(EVisibility::Collapsed);
	InteractiveButton->DisableInput();
	InteractiveButton->ForceDeactivate();
}

void SPlayerHUD::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (HitMarkDisplayTime.IsActive())
	{
		HitMarkDisplayTime.InterpConstantTo(InDeltaTime);

		if (HitMarkDisplayTime.IsCompleted())
		{
			HiddenHitMark();
		}
	}

	if (HealthBackgroundAnim.IsActive())
	{
		HealthBackgroundAnim.ConstantToAndDeactivate(InDeltaTime);
		HealthBackgroundBar->SetPercent(HealthBackgroundAnim.GetValue());
	}

	if (HealthRecoveryAnim.IsActive())
	{
		HealthRecoveryAnim.InterpConstantTo(InDeltaTime);
		HealthBarPercent = HealthRecoveryAnim.GetValue();
		HealthBar->SetPercent(HealthBarPercent);
		
		if (HealthRecoveryAnim.IsCompleted())
		{
			HealthBackgroundBar->SetPercent(HealthRecoveryAnim.GetValue());
			HealthRecoveryAnim.Deactivate();
		}
	}

	HurtAnim.TickTimeline(InDeltaTime);
}

void SPlayerHUD::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(HurtDynamic);
}

#undef LOCTEXT_NAMESPACE