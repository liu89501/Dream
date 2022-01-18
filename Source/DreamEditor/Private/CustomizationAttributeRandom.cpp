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
		const FItemDef& Definition = UDProjectSettings::GetProjectSettings()->GetItemDefinition(Guid);
		SelectedOption = MakeShared<FComboboxItem>(Definition.ItemBaseInfo.PropsName, Guid);
	}
}

void FItemSelectedCustomization::OnOpeningCombobox()
{
	TArray<FItemDetails> Items;

	GetAllItems(Items);

	Options.Reset();

	for (const FItemDetails& Item : Items)
	{
		Options.Add(MakeShared<FComboboxItem>(Item.ItemName, Item.ItemGuid));
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
	return !SelectedOption.IsValid() || SelectedOption->ItemName.IsEmpty()
		       ? LOCTEXT("SelecteAbility", "选择物品")
		       : FormatItemText(SelectedOption);
}

FText FItemSelectedCustomization::FormatItemText(TSharedPtr<FComboboxItem> Item) const
{
	return FText::Format(FTextFormat::FromString(TEXT("{0} - {1}")), UEnum::GetDisplayValueAsText(GetItemType(Item->Guid)), Item->ItemName);
}

bool FItemSelectedCustomization::GetAllItems(TArray<FItemDetails>& Items) const
{
	if (UDataTable* ItemTable = UDProjectSettings::GetProjectSettings()->GetItemTable())
	{
		EItemType::Type ItemType = static_cast<EItemType::Type>(FilterType);

		for (TTuple<FName, uint8*> Row : ItemTable->GetRowMap())
		{
			int32 ItemGuid = FCString::Atoi(*Row.Key.ToString());
			EItemType::Type Type = GetItemType(ItemGuid);

			if (Type == ItemType || ItemType == EItemType::All)
			{
				FItemDef* Def = reinterpret_cast<FItemDef*>(Row.Value);
				FItemDetails Details;
				Details.ItemName = Def->ItemBaseInfo.PropsName;
				Details.ItemGuid = ItemGuid;
				Items.Add(Details);
			}
		}

		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
