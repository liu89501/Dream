#pragma once

#include "CoreMinimal.h"

class FDreamEditorModule : public IModuleInterface
{

public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:

	TSharedPtr<class FWeaponMeshPreviewCustomizationMenu> WeaponMeshPreview;
	
	TSharedPtr<class FItemsTableMenu> ItemsTableMenu;

	TSharedPtr<class FGearAttributesGraphPinFactory> GearAttributesGraphPinFactory;
};
