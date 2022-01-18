#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

struct FComboboxItem
{
	FComboboxItem()
		: Guid(0)
	{
	}
	
	FComboboxItem(const FText& InItemName, int32 InGuid)
		: ItemName(InItemName)
		, Guid(InGuid)
	{
	}
	
	FText ItemName;
	int32 Guid;
};

struct FItemDetails
{
	int32 ItemGuid;
	FText ItemName;
};

class FItemSelectedCustomization : public IPropertyTypeCustomization
{

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

private:

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	void InitializeSelectedOption();

	TSharedRef<SWidget> OnGenerateOptionsWidget(TSharedPtr<FComboboxItem> Option) const;
	void OnOptionSelectedChange(TSharedPtr<FComboboxItem> Option, ESelectInfo::Type SelectInfo);
	void OnOpeningCombobox();

	FText GetComboboxSelectedText() const;

	FText FormatItemText(TSharedPtr<FComboboxItem> Item) const;

	bool GetAllItems(TArray<FItemDetails>& Items) const;

private:

	int32 FilterType;

	TArray<TSharedPtr<FComboboxItem>> Options;
	TSharedPtr<FComboboxItem> SelectedOption;

private:

	TSharedPtr<IPropertyHandle> GuidHandle;
};

