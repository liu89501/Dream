// ReSharper disable All
#include "DCustomizationEditorTools.h"


#include "ConstructorHelpers.h"
#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "ShootWeapon.h"
#include "SUniformGridPanel.h"
#include "Engine/SkeletalMeshSocket.h"

#define LOCTEXT_NAMESPACE "CustomizationEditorTools"

TSharedRef<IPropertyTypeCustomization> FWeaponPreviewCustomization::MakeInstance()
{
	return MakeShareable(new FWeaponPreviewCustomization);
}

void FWeaponPreviewCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> SPropertyHandle,
                                                    IDetailChildrenBuilder& ChildBuilder,
                                                    IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FWeaponPreviewCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> SPropertyHandle,
                                                  FDetailWidgetRow& HeaderRow,
                                                  IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyHandle = SPropertyHandle;

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SBox)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(2.0f)
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
	                .OnClicked(this, &FWeaponPreviewCustomization::OnButtonClicked)
	                .HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("附加到预览骨骼")))
					]
				]
			]
		];
}

FReply FWeaponPreviewCustomization::OnButtonClicked()
{
	TArray<UObject*> OuterObjs; 
	PropertyHandle->GetOuterObjects(OuterObjs);

	UE_LOG(LogTemp, Log, TEXT("OnButtonClicked"));
	
	return FReply::Handled();
}

void FWeaponMeshPreviewCustomizationMenu::LoadMenuContext()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	TArray<FContentBrowserMenuExtender_SelectedAssets> & CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FWeaponMeshPreviewCustomizationMenu::ProcessMenuContext));
}

void FWeaponMeshPreviewCustomizationMenu::AddContextMenuEntries(FMenuBuilder& MenuBuilder) const
{
	MenuBuilder.AddMenuEntry(
        LOCTEXT("MenuLabel", "添加到预览网格体"),
        LOCTEXT("MenuLabel", "添加到预览网格体"),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateSP(this, &FWeaponMeshPreviewCustomizationMenu::OnExecuteAction),
            FCanExecuteAction::CreateSP(this, &FWeaponMeshPreviewCustomizationMenu::OnCanExecuteAction)
        )
    );
}

void FWeaponMeshPreviewCustomizationMenu::OnExecuteAction() const
{
	AShootWeapon* WeaponCDO = SelectedWeaponClass->GetDefaultObject<AShootWeapon>();

	static FString SkeletonAssetPath = TEXT("/Game/Character/BaseCharacter/Mesh/UE4_Mannequin_Skeleton");

	USkeleton* Skeleton = FindObject<USkeleton>(NULL, *SkeletonAssetPath);

	if (Skeleton == nullptr)
	{
		Skeleton = LoadObject<USkeleton>(NULL, *SkeletonAssetPath);
	}
	
	if (Skeleton)
	{
		static FName PreviewSocketName(TEXT("Preview"));
		USkeletalMeshSocket* PreviewSocket = Skeleton->FindSocket(PreviewSocketName);

		PreviewSocket->RelativeLocation = WeaponCDO->WeaponSocketOffset.GetLocation();
		PreviewSocket->RelativeRotation = WeaponCDO->WeaponSocketOffset.Rotator();
		PreviewSocket->RelativeScale = WeaponCDO->WeaponSocketOffset.GetScale3D();
		Skeleton->PreviewAttachedAssetContainer.ClearAllAttachedObjects();
		Skeleton->PreviewAttachedAssetContainer.AddAttachedObject(WeaponCDO->WeaponMesh->SkeletalMesh, PreviewSocketName);
		Skeleton->Modify();
	}
}

bool FWeaponMeshPreviewCustomizationMenu::OnCanExecuteAction() const
{
	return SelectedWeaponClass != nullptr;
}

TSharedRef<class FExtender> FWeaponMeshPreviewCustomizationMenu::ProcessMenuContext(const TArray<FAssetData> & NewSelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	bool bAnySupportedAssets = false;
	for (const FAssetData& Asset : NewSelectedAssets)
	{
		if (Asset.GetClass() != UBlueprint::StaticClass())
		{
			continue;
		}
		//Asset.PrintAssetData();

		FString ClassName = Asset.ObjectPath.ToString().Append("_C");
		if (UClass* Class = LoadClass<AShootWeapon>(nullptr, *ClassName))
		{
			SelectedWeaponClass = Class;
			bAnySupportedAssets = true;
			break;
		}
	}

	if (bAnySupportedAssets)
	{
		Extender->AddMenuExtension(
            "GetAssetActions",
            EExtensionHook::After,
            nullptr,
            FMenuExtensionDelegate::CreateSP(this, &FWeaponMeshPreviewCustomizationMenu::AddContextMenuEntries)
        );
	}

	return Extender;
}
