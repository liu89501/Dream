// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DreamType.h"
#include "Engine/GameViewportClient.h"
#include "DMViewportClient.generated.h"

class SSubtitle;
class SDialog;
enum class EDialogType : uint8;

/**
 * 
 */
UCLASS()
class DREAM_API UDMViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:

	UDMViewportClient();
	
	void PopupDialog(const FText& Content, EDialogType DialogType, float DisplayTime);

	void DismissDialog();

	void DisplaySubtitle(const FText& SubtitleText);

protected:
	
	virtual void Tick(float DeltaTime) override;

	virtual	void BeginDestroy() override;
	
	virtual void DetachViewportClient() override;

	void OnDMViewportCreated();

	void ReleaseSlateWidgets();
	
private:

	TSharedPtr<SDialog> DialogWidget;
	FTimerTemplate DisplayTimer;

	TSharedPtr<SSubtitle> SubtitleWidget;
	bool bAddSubtitleWidget;
};

