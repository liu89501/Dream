#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class FWeaponPreviewCustomization : public IPropertyTypeCustomization
{
	
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
private:

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	FReply OnButtonClicked();

	TSharedPtr<IPropertyHandle> PropertyHandle;
};


class FWeaponMeshPreviewCustomizationMenu : public TSharedFromThis<FWeaponMeshPreviewCustomizationMenu>
{
public:

	UClass* SelectedWeaponClass;

	void LoadMenuContext();

	TSharedRef<class FExtender> ProcessMenuContext(const TArray<FAssetData> & NewSelectedAssets);
	void AddContextMenuEntries(class FMenuBuilder& MenuBuilder) const;

	void OnExecuteAction() const;
	bool OnCanExecuteAction() const;
};