// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterface.h"
#include "SlateUtils.h"
#include "Style/InteractiveWidgetStyle.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnInteractiveCompleted)

/**
 * 
 */
class SInteractive : public SCompoundWidget, FGCObject
{
	
public:

	SLATE_BEGIN_ARGS(SInteractive)
	{}
		SLATE_ARGUMENT(APlayerController*, InPlayerOwner);
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void EnableInput() const;
	void DisableInput() const;

	void SetOnCompleted(FOnInteractiveCompleted Delegate);
	void SetInteractiveTime(float InInteractiveTime);

	void ForceDeactivate();

	void SetInteractiveText(const FText& InteractiveText) const;
	
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	~SInteractive();

protected:

	void InitializeInputComponent();
	void DestroyInputComponent();

private:

	void OnInteractiveButtonPressed();
	void OnInteractiveButtonReleased();

	void UpdateProgress() const;

private:

	FLinearAnimation InteractiveTime;

	TSharedPtr<STextBlock> HintText;
	
	TWeakObjectPtr<APlayerController> PlayerOwner;

	UInputComponent* InputComponent;

	FSlateBrush InProgressBrush;

	UMaterialInstanceDynamic* PressedInProgress;

	FOnInteractiveCompleted OnCompleted;

	FInteractiveStyle InteractiveStyle;

};
