// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"

/**
 * 
 */
class SGearAttributesPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SGearAttributesPin)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	virtual TSharedRef<SWidget> GetDefaultValueWidget() override;

	void OnAttributeChanged(FFloatProperty* SelectedAttribute);

	bool GetIsEnable() const;
};
