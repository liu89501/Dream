#include "CustomizationAttributeRandom.h"
#include "DetailWidgetRow.h"
#include "DProjectSettings.h"
#include "DPropsType.h"
#include "SComboBox.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "CustomizationAttributeRandom"

TSharedRef<IPropertyTypeCustomization> FItemSelectedCustomization::MakeInstance()
{
	return MakeShared<FItemSelectedCustomization>();
}

void FItemSelectedCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
                                                   IDetailChildrenBuilder& ChildBuilder,
                                                   IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FItemSelectedCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
                                                 FDetailWidgetRow& HeaderRow,
                                                 IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	//UE_LOG(LogTemp, Error, TEXT("CustomizeHeader"));

	FilterType = EItemType::All;
	
	FString FilterItemType = PropertyHandle->GetMetaData("FilterItemType");

	if (!FilterItemType.IsEmpty())
	{
		if (UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EItemType")))
		{
			int64 EnumValue = Enum->GetValueByName(*FilterItemType);
			if (EnumValue != INDEX_NONE)
			{
				FilterType = EnumValue;
			}
		}
	}

	GuidHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemGuidHandle, ItemGuid));

	InitializeSelectedOption();

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SComboBox<TSharedPtr<FComboboxItem>>)
		    .OptionsSource(&Options)
		    .OnGenerateWidget_Raw(this, &FItemSelectedCustomization::OnGenerateOptionsWidget)
		    .OnSelectionChanged_Raw(this, &FItemSelectedCustomization::OnOptionSelectedChange)
		    .OnComboBoxOpening_Raw(this, &FItemSelectedCustomization::OnOpeningCombobox)
		    .ContentPadding(2.0f)
		    .Content()
			[
				SNew(STextBlock)
				.Text(this, &FItemSelectedCustomization::GetComboboxSelectedText)
			]
		];
}

void FItemSelectedCustomization::InitializeSelectedOption()
{
	int32 Guid;
	if (GuidHandle->GetValue(Guid) == FPropertyAccess::Success)
	{
		const FItemDefinition& Definition = UDProjectSettings::GetProjectSettings()->GetItemDefinition(Guid);

		int32 Index;
		if (Definition.ItemClass.FindLastChar(TEXT('.'), Index))
		{
			SelectedOption = MakeShared<FComboboxItem>(Definition.ItemClass.RightChop(Index + 1), Guid);
		}
	}
}

void FItemSelectedCustomization::OnOpeningCombobox()
{
	TArray<FItemDetails> Items;

	UDProjectSettings::GetProjectSettings()->GetAllItems(static_cast<EItemType::Type>(FilterType), Items);

	Options.Reset();

	for (const FItemDetails& Item : Items)
	{
		int32 Index;
		if (Item.ItemClass.FindLastChar(TEXT('.'), Index))
		{
			Options.Add(MakeShared<FComboboxItem>(Item.ItemClass.RightChop(Index + 1), Item.ItemGuid));
		}
	}
}

TSharedRef<SWidget> FItemSelectedCustomization::OnGenerateOptionsWidget(TSharedPtr<FComboboxItem> Option) const
{
	return SNew(STextBlock).Text(FormatItemText(Option));
}

void FItemSelectedCustomization::OnOptionSelectedChange(TSharedPtr<FComboboxItem> Option, ESelectInfo::Type SelectInfo)
{
	if (Option.IsValid())
	{
		SelectedOption = Option;
		GuidHandle->SetValue(Option->Guid);
	}
}

FText FItemSelectedCustomization::GetComboboxSelectedText() const
{
	return !SelectedOption.IsValid() || SelectedOption->Text.IsEmpty()
		       ? LOCTEXT("SelecteAbility", "选择物品")
		       : FormatItemText(SelectedOption);
}

FText FItemSelectedCustomization::FormatItemText(TSharedPtr<FComboboxItem> Item) const
{
	return FText::FromString(FString::Printf(TEXT("%s - %s"), *UEnum::GetDisplayValueAsText(GetItemType(Item->Guid)).ToString(), *Item->Text));
}

#undef LOCTEXT_NAMESPACE
