// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserWidget.h"
#include "DMTeamViewer.generated.h"


/**
 * 
 */
UCLASS()
class UDMTeamViewer : public UUserWidget
{
	GENERATED_BODY()

	UDMTeamViewer(const FObjectInitializer& ObjectInitializer);

public:


public:


protected:

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

};

