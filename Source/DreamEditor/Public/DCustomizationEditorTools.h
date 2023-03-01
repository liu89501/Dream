#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class FTargetDelegateCustomization : public IPropertyTypeCustomization
{
	
public:

	FTargetDelegateCustomization() = default;
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
private:

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	void OnComboBoxSelectedItemChange(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo);
	TSharedRef<class SWidget> OnGenerateComboBoxItem(TSharedPtr<FName> InItem);
	FText GetItemText() const;
	FMulticastDelegateProperty* GetTargetDelegateProperty();

	FText GetFuncButtonText() const;
	void OnBindingBtnClick();
	void OnRemoveBtnClick();
	void OnBindingFuncSelected(FName SelectedFuncName);
	TSharedRef<SWidget> OnGenerateDelegateMenu();
	bool IsValidBindFunction(UClass* Class, const FName& FuncName);
	bool IsTargetDelegateModify();
	bool IsTriggerFunctionModify();

	void RefreshData();

	void OnTargetActorChanged();

	//TSharedPtr<FName> BindingFuncName;
	UBlueprint* Blueprint;

	TSharedPtr<IPropertyHandle> TargetActorHandle;
	TSharedPtr<IPropertyHandle> TargetDelegateHandle;
	TSharedPtr<IPropertyHandle> TriggerFunctionHandle;
	TArray<TSharedPtr<FName>> OptionSources;
};


class FWeaponMeshPreviewCustomizationMenu : public TSharedFromThis<FWeaponMeshPreviewCustomizationMenu>
{
	
public:

	void LoadMenuContext();
	TSharedRef<class FExtender> ProcessMenuContext(const TArray<FAssetData> & NewSelectedAssets);
	void AddContextMenuEntries(class FMenuBuilder& MenuBuilder) const;

	bool OnCanExecuteAction() const;
	void OnExecuteAction() const;
	void OnExecuteAction2() const;
	void OnExecuteAction3() const;
	void OnExecuteAction4() const;

private:

	TWeakObjectPtr<class AShootWeapon> Weapon;
};


class FItemsTableMenu
{

public:

	void LoadMenuContext();
	void AddContextMenuEntries(class FMenuBuilder& MenuBuilder) const;
	TSharedRef<class FExtender> ProcessMenuContext(const TArray<FAssetData> & NewSelectedAssets);

	void OnExecuteRepair() const;
	bool OnCanExecuteRepair() const;

private:
	
	TWeakObjectPtr<UDataTable> DataTable;
	
};