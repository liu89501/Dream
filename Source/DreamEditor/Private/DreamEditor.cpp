
#include "DreamEditor.h"
#include "CustomizationAttributeRandom.h"
#include "DCustomizationEditorTools.h"
#include "DProjectSettings.h"
#include "PreviewSceneSettings.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "DreamEditor"

void FDreamEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("TargetDelegate"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FTargetDelegateCustomization::MakeInstance));
	
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("ItemGuidHandle"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemSelectedCustomization::MakeInstance));

	WeaponMeshPreview = MakeShareable(new FWeaponMeshPreviewCustomizationMenu());
	WeaponMeshPreview->LoadMenuContext();

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	SettingsModule->RegisterSettings("Project", "Project", "Preview",
            LOCTEXT("PreviewSceneSettingsName", "预览场景设置"),
            LOCTEXT("PreviewSceneSettingsDescription", "预览场景设置"),
            GetMutableDefault<UPreviewSceneSettings>()
        );
	
	SettingsModule->RegisterSettings("Project", "Project", "Dream",
            LOCTEXT("GameProjectSettingsName", "当前游戏项目设置"),
            LOCTEXT("GameProjectSettingsDescription", "当前游戏项目设置"),
            GetMutableDefault<UDProjectSettings>()
        );

}

void FDreamEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("TargetDelegate"));
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("ItemGuidHandle"));

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	SettingsModule->UnregisterSettings("Project", "Project", "Preview");
	SettingsModule->UnregisterSettings("Project", "Project", "Dream");
}

bool FDreamEditorModule::IsGameModule() const
{
	return false;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE(FDreamEditorModule, DreamEditor);
