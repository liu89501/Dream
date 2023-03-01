
#include "DreamEditor.h"
#include "CustomizationAttributeRandom.h"
#include "DCustomizationEditorTools.h"
#include "DMDevelopmentSettings.h"
#include "DMProjectSettings.h"
#include "EdGraphUtilities.h"
#include "FGearAttributesGraphPinFactory.h"
#include "PreviewSceneSettings.h"
#include "ISettingsModule.h"
#include "PropertyCustomizeGearAttribute.h"

#define LOCTEXT_NAMESPACE "DreamEditor"

void FDreamEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("TargetDelegate"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FTargetDelegateCustomization::MakeInstance));
	
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("ItemGuidHandle"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemSelectedCustomization::MakeInstance));
	
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("AttributeHandle"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomizeGearAttribute::MakeInstance));

	WeaponMeshPreview = MakeShared<FWeaponMeshPreviewCustomizationMenu>();
	WeaponMeshPreview->LoadMenuContext();

	ItemsTableMenu = MakeShared<FItemsTableMenu>();
	ItemsTableMenu->LoadMenuContext();

	GearAttributesGraphPinFactory = MakeShared<FGearAttributesGraphPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(GearAttributesGraphPinFactory);

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
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("AttributeHandle"));

	FEdGraphUtilities::UnregisterVisualPinFactory(GearAttributesGraphPinFactory);

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
