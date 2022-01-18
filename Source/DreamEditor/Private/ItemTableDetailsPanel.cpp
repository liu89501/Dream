#include "ItemTableDetailsPanel.h"

#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "ItemTableDetailsPanel"

TSharedRef<IDetailCustomization> FItemTableDetailsPanel::MakeInstance()
{
	return MakeShared<FItemTableDetailsPanel>();
}

void FItemTableDetailsPanel::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	/*TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(TEXT("Node.Items"));
	DetailBuilder.AddCustomRowToCategory(PropertyHandle, LOCTEXT("ItemSearch", "Item"))
	[
		
	];*/
	
}

#undef LOCTEXT_NAMESPACE

