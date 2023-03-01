// Fill out your copyright notice in the Description page of Project Settings.

#include "SGearAttributesWidget.h"
#include "DPropsType.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGearAttributesWidget::Construct(const FArguments& InArgs)
{
	SelectedProperty = InArgs._DefaultSelectedProperty;
	OnAttributeChange = InArgs._OnAttributeChange;
	
	UpdateAttributesList();
	
	ChildSlot
	[
		SNew(SComboBox<TFieldPath<FFloatProperty>>)
        .OptionsSource(&AttributeList)
        .OnGenerateWidget_Raw(this, &SGearAttributesWidget::OnGenerateOptionsWidget)
        .OnSelectionChanged_Raw(this, &SGearAttributesWidget::OnOptionSelectedChange)
        .ContentPadding(2.0f)
        .Content()
        [
            SNew(STextBlock)
            .Text(this, &SGearAttributesWidget::GetComboboxSelectedText)
        ]
	];
}

TSharedRef<SWidget> SGearAttributesWidget::OnGenerateOptionsWidget(TFieldPath<FFloatProperty> Option) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Option->GetName()));
}

void SGearAttributesWidget::OnOptionSelectedChange(TFieldPath<FFloatProperty> Option, ESelectInfo::Type SelectInfo)
{
	SelectedProperty = Option.Get();
	OnAttributeChange.ExecuteIfBound(SelectedProperty);
}

FText SGearAttributesWidget::GetComboboxSelectedText() const
{
	return SelectedProperty ? FText::FromString(SelectedProperty->GetName()) : FText::FromString(TEXT("请选择属性"));
}

void SGearAttributesWidget::UpdateAttributesList()
{
	AttributeList.Reset();

	TFieldIterator<FFloatProperty> Iterator(FEquipmentAttributes::StaticStruct(), EFieldIteratorFlags::ExcludeSuper);

	for ( ; Iterator; ++Iterator)
	{
		if (FFloatProperty* Property = CastField<FFloatProperty>(*Iterator))
		{
			AttributeList.Add(Property);
		}
	}
}

FFloatProperty* SGearAttributesWidget::GetSelectedProperty() const
{
	return SelectedProperty;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
