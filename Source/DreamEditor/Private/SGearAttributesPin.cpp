// Fill out your copyright notice in the Description page of Project Settings.


#include "SGearAttributesPin.h"

#include "DPropsType.h"
#include "SGearAttributesWidget.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGearAttributesPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct( SGraphPin::FArguments(), InGraphPinObj );
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


TSharedRef<SWidget> SGearAttributesPin::GetDefaultValueWidget()
{
	FString DefaultString = GraphPinObj->GetDefaultAsString();

	FFloatProperty* DefaultProperty = nullptr;

	if (!DefaultString.IsEmpty())
	{
		UScriptStruct* ScriptStruct = FAttributeHandle::StaticStruct();

		FAttributeHandle Attribute;
		ScriptStruct->ImportText(*DefaultString, &Attribute, nullptr, EPropertyPortFlags::PPF_SerializedAsImportText, GError, ScriptStruct->GetName());

		DefaultProperty = Attribute.AttributeProperty.Get();
	}

	return SNew(SGearAttributesWidget)
            .OnAttributeChange(this, &SGearAttributesPin::OnAttributeChanged)
            .DefaultSelectedProperty(CastField<FFloatProperty>(DefaultProperty))
			.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
			.IsEnabled(this, &SGearAttributesPin::GetIsEnable);
}

void SGearAttributesPin::OnAttributeChanged(FFloatProperty* SelectedAttribute)
{
	FString ExportText;
	FAttributeHandle Attribute;
	Attribute.AttributeProperty = SelectedAttribute;
	FAttributeHandle::StaticStruct()->ExportText(ExportText, &Attribute, &Attribute, nullptr, EPropertyPortFlags::PPF_SerializedAsImportText, nullptr);

	if (ExportText != GraphPinObj->GetDefaultAsString())
	{
		const FScopedTransaction Transaction(FText::FromString(TEXT("Change Pin Value")));
		GraphPinObj->Modify();
		GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, ExportText);
	}
}

bool SGearAttributesPin::GetIsEnable() const
{
	return GraphPinObj ? !GraphPinObj->bDefaultValueIsReadOnly : true;
}
