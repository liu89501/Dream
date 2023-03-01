#include "PropertyCustomizeGearAttribute.h"
#include "DetailWidgetRow.h"
#include "DPropsType.h"
#include "IDetailChildrenBuilder.h"
#include "SGearAttributesWidget.h"

#define LOCTEXT_NAMESPACE "PropertyCustomizeGearAttribute"

TSharedRef<IPropertyTypeCustomization> FPropertyCustomizeGearAttribute::MakeInstance()
{
	return MakeShared<FPropertyCustomizeGearAttribute>();
}

void FPropertyCustomizeGearAttribute::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
                                                   IDetailChildrenBuilder& ChildBuilder,
                                                   IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FPropertyCustomizeGearAttribute::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
                                                 FDetailWidgetRow& HeaderRow,
                                                 IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	//UE_LOG(LogTemp, Error, TEXT("CustomizeHeader"));

	AttributeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAttributeHandle, AttributeProperty));

	FProperty* Property;
	AttributeHandle->GetValue(Property);

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.HAlign(HAlign_Left)
		.MinDesiredWidth(200.f)
		[
			SNew(SGearAttributesWidget)
		    .OnAttributeChange_Raw(this, &FPropertyCustomizeGearAttribute::OnOptionSelectedChange)
		    .DefaultSelectedProperty(CastField<FFloatProperty>(Property))
		];
}

void FPropertyCustomizeGearAttribute::OnOptionSelectedChange(FFloatProperty* Attribute)
{
	if (Attribute)
	{
		AttributeHandle->SetValue(Attribute);
	}
}

#undef LOCTEXT_NAMESPACE
