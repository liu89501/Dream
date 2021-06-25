#pragma once

#include "CoreMinimal.h"

class FDreamEditorModule : public IModuleInterface
{

public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override;

private:

	TSharedPtr<class FWeaponMeshPreviewCustomizationMenu> WeaponMeshPreview;
};
