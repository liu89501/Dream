
#include "DreamEditor.h"
#include "DCustomizationEditorTools.h"

void FDreamEditorModule::StartupModule()
{
	/*FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("WeaponAttachToPreviewSkeletal"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWeaponPreviewCustomization::MakeInstance));*/

	WeaponMeshPreview = MakeShareable(new FWeaponMeshPreviewCustomizationMenu());
	WeaponMeshPreview->LoadMenuContext();
}

void FDreamEditorModule::ShutdownModule()
{
	/*FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("WeaponAttachToPreviewSkeletal"));*/
}

bool FDreamEditorModule::IsGameModule() const
{
	return false;
}

IMPLEMENT_GAME_MODULE(FDreamEditorModule, DreamEditor);
