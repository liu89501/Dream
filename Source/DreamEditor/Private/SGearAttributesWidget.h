// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_OneParam(FOnAttributeChange, FFloatProperty*);

/**
 * 
 */
class SGearAttributesWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGearAttributesWidget)
	{}

		SLATE_ARGUMENT(FFloatProperty*, DefaultSelectedProperty);

		SLATE_EVENT(FOnAttributeChange, OnAttributeChange);
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	TSharedRef<SWidget> OnGenerateOptionsWidget(TFieldPath<FFloatProperty> Option) const;
	void OnOptionSelectedChange(TFieldPath<FFloatProperty> Option, ESelectInfo::Type SelectInfo);

	FText GetComboboxSelectedText() const;

	void UpdateAttributesList();

	FFloatProperty* GetSelectedProperty() const;

private:

	FOnAttributeChange OnAttributeChange;
	
	TArray<TFieldPath<FFloatProperty>> AttributeList;

	FFloatProperty* SelectedProperty;
};
