// Fill out your copyright notice in the Description page of Project Settings.

#include "DMViewportClient.h"
#include "UI/SDialog.h"
#include "UI/SSubtitle.h"

#define SlateWidgetZOrder(Enum) (EWidgetOrder::Enum + 10)

UDMViewportClient::UDMViewportClient()
	: bAddSubtitleWidget(false)
{
	UGameViewportClient::OnViewportCreated().AddUObject(this, &UDMViewportClient::OnDMViewportCreated);
}

void UDMViewportClient::PopupDialog(const FText& Content, EDialogType DialogType, float DisplayTime)
{
	DismissDialog();

	DialogWidget->UpdateDialog(DialogType, Content);
	
	DisplayTimer.Reset(DisplayTime);
	DisplayTimer.Resume();
	DialogWidget->PlayFadeInAnim(Viewport->GetSizeXY().X);
	AddViewportWidgetContent(DialogWidget.ToSharedRef(), SlateWidgetZOrder(PlayerCtrlPopup));
}

void UDMViewportClient::DismissDialog()
{
	if (DisplayTimer.IsInProgress())
	{
		RemoveViewportWidgetContent(DialogWidget.ToSharedRef());
		DisplayTimer.Pause();
	}
}

void UDMViewportClient::DisplaySubtitle(const FText& SubtitleText)
{
	if (!bAddSubtitleWidget)
	{
		bAddSubtitleWidget = true;
		AddViewportWidgetContent(SubtitleWidget.ToSharedRef(), SlateWidgetZOrder(Subtitle));
	}
	
	SubtitleWidget->DisplaySubtitle(SubtitleText);
	SubtitleWidget->SetVisibility(EVisibility::SelfHitTestInvisible);
}

void UDMViewportClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!DisplayTimer.IsPause())
	{
		if (DisplayTimer.Tick(DeltaTime))
		{
			DialogWidget->SetRenderTransform(FTransform2D(FVector2D(-Viewport->GetSizeXY().X, 0)));
			RemoveViewportWidgetContent(DialogWidget.ToSharedRef());
			DisplayTimer.Pause();
        }
	}
}

void UDMViewportClient::BeginDestroy()
{
	Super::BeginDestroy();

	ReleaseSlateWidgets();
}

void UDMViewportClient::DetachViewportClient()
{
	Super::DetachViewportClient();

	ReleaseSlateWidgets();
}

void UDMViewportClient::OnDMViewportCreated()
{
	DialogWidget = SNew(SDialog)
		.Visibility(EVisibility::SelfHitTestInvisible);
	
	SubtitleWidget = SNew(SSubtitle)
        .DisplayMode(EDisplayMode::OnePerFrame)
        .Visibility(EVisibility::Collapsed);
}

void UDMViewportClient::ReleaseSlateWidgets()
{
	DialogWidget.Reset();
	SubtitleWidget.Reset();
}
