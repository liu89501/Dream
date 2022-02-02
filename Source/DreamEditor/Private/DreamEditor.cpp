
#include "DreamEditor.h"
#include "CustomizationAttributeRandom.h"
#include "DCustomizationEditorTools.h"
#include "DMDevelopmentSettings.h"
#include "DMProjectSettings.h"
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
	if (SettingsModule != nullptr)
	{
		SettingsModule->RegisterSettings("Project", "Project", "Preview",
                LOCTEXT("PreviewSceneSettingsName", "预览场景设置"),
                LOCTEXT("PreviewSceneSettingsDescription", "预览场景设置"),
                GetMutableDefault<UPreviewSceneSettings>()
            );
	
		SettingsModule->RegisterSettings("Project", "Project", "Dream",
                LOCTEXT("GameProjectSettingsName", "游戏相关设置"),
                LOCTEXT("GameProjectSettingsDescription", "游戏相关设置"),
                GetMutableDefault<UDMProjectSettings>()
            );
            
		SettingsModule->RegisterSettings("Project", "Project", "DMDevelop",
                LOCTEXT("DevSettingsName", "开发相关设置"),
                LOCTEXT("DevSettingsDescription", "开发相关设置"),
                GetMutableDefault<UDMDevelopmentSettings>()
            );
	}
}

void FDreamEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("TargetDelegate"));
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("ItemGuidHandle"));

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Project", "Preview");
		SettingsModule->UnregisterSettings("Project", "Project", "Dream");
		SettingsModule->UnregisterSettings("Project", "Project", "DMDevelop");
	}
}

IMPLEMENT_MODULE(FDreamEditorModule, DreamEditor);

#undef LOCTEXT_NAMESPACE
