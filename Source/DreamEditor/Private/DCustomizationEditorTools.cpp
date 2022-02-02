// ReSharper disable All
#include "DCustomizationEditorTools.h"
#include "BlueprintEditorModule.h"
#include "BlueprintEditorUtils.h"
#include "BroadcastReceiverComponent.h"
#include "ConstructorHelpers.h"
#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DMDevelopmentSettings.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyAccessEditor.h"
#include "IPropertyUtilities.h"
#include "KismetEditorUtilities.h"
#include "ShootWeapon.h"
#include "Widgets/Input/SComboBox.h"
#include "SUniformGridPanel.h"
#include "Engine/SkeletalMeshSocket.h"

#define LOCTEXT_NAMESPACE "CustomizationEditorTools"

struct FParamInformation
{
	FProperty* Property;
	bool bIsReturnParam;
};

TSharedRef<IPropertyTypeCustomization> FTargetDelegateCustomization::MakeInstance()
{
	return MakeShareable(new FTargetDelegateCustomization);
}

void FTargetDelegateCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> SPropertyHandle,
                                                    IDetailChildrenBuilder& ChildBuilder,
                                                    IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ChildBuilder.AddProperty(TargetActorHandle.ToSharedRef());

	RefreshData();

	ChildBuilder.AddCustomRow(LOCTEXT("DelegateName", "Delegate"))
	.DiffersFromDefault(true)
	.NameContent()
    [
		SPropertyHandle->CreatePropertyNameWidget(LOCTEXT("DelegateName", "Delegate"), LOCTEXT("TargetActorDelegateName", "TargetActorDelegate"))
    ]
    .ValueContent()
    .HAlign(HAlign_Fill)
    [
        SNew(SBox)
        [
            SNew(SUniformGridPanel)
            .SlotPadding(FMargin(0, 0, 20, 0))
            + SUniformGridPanel::Slot(0, 0)
            [
                SNew(SComboBox<TSharedPtr<FName>>)
                .OptionsSource(&OptionSources)
                .OnGenerateWidget(this, &FTargetDelegateCustomization::OnGenerateComboBoxItem)
                .OnSelectionChanged(this, &FTargetDelegateCustomization::OnComboBoxSelectedItemChange)
                .Content()
                [
					SNew(STextBlock).Text(this, &FTargetDelegateCustomization::GetItemText)
                ]
            ]
        ]
    ];
	
		
	ChildBuilder.AddCustomRow(LOCTEXT("FuncName", "Function"))
	.DiffersFromDefault(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateRaw(this, &FTargetDelegateCustomization::IsTriggerFunctionModify)))
    .NameContent()
    [
        SPropertyHandle->CreatePropertyNameWidget(
            LOCTEXT("FuncName", "Function"), LOCTEXT("FuncTooltips", "目标委托触发时调用的函数"))
    ]
    .ValueContent()
    .HAlign(HAlign_Fill)
    [
        SNew(SOverlay)
        + SOverlay::Slot()
        .Padding(FMargin(0, 0, 20, 0))
        [
            SNew(SComboButton)
            .ToolTipText(LOCTEXT("ToolTip", "绑定到自身触发的函数"))
            .OnGetMenuContent(this, &FTargetDelegateCustomization::OnGenerateDelegateMenu)
            .ContentPadding(1)
            .ButtonContent()
            [
                SNew(SHorizontalBox)
                .Clipping(EWidgetClipping::ClipToBounds)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(SBox)
                    .HeightOverride(16.0f)
                    [
                        SNew(SImage)
                        .Image(nullptr)
                        .ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f))
                    ]
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(4, 0, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(this, &FTargetDelegateCustomization::GetFuncButtonText)
                    .Font(IDetailLayoutBuilder::GetDetailFont())
                ]
            ]
        ]
    ];
}

void FTargetDelegateCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> SPropertyHandle,
                                                  FDetailWidgetRow& HeaderRow,
                                                  IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TargetActorHandle = SPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTargetDelegate, Target));
	TargetDelegateHandle = SPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTargetDelegate, DelegateName));
	TriggerFunctionHandle = SPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTargetDelegate, TriggerFunctionName));

	TargetActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateRaw(this, &FTargetDelegateCustomization::OnTargetActorChanged));

	TArray<UObject*> OuterObjs;
	SPropertyHandle->GetOuterObjects(OuterObjs);
	if (OuterObjs.IsValidIndex(0))
	{
		UBlueprint* TargetBP = nullptr;
		for (UObject* TestObject = OuterObjs[0]; (TestObject != nullptr) && (TargetBP == nullptr); TestObject = TestObject->GetOuter())
		{
			if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(TestObject->GetClass()))
			{
				TargetBP = Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
			}
		}
		Blueprint = TargetBP;
	}
}

void FTargetDelegateCustomization::RefreshData()
{
	UObject* TargetActor;
	FPropertyAccess::Result Result = TargetActorHandle->GetValue(TargetActor);

	if (Result != FPropertyAccess::Success || TargetActor == nullptr)
	{
		return;
	}

	/* 初始化下拉列表 */
	OptionSources.Reset();
	for (TFieldIterator<FMulticastDelegateProperty> It(TargetActor->GetClass()); It; ++It)
	{
		OptionSources.Add(MakeShareable(new FName(It->GetFName())));
	}
	
	FName TargetDelegateName;
	FPropertyAccess::Result DelegateValueResult = TargetDelegateHandle->GetValue(TargetDelegateName);
	if (DelegateValueResult == FPropertyAccess::Success && !TargetDelegateName.IsNone())
	{
		bool bResetTargetDelegate = false;

		if (Result == FPropertyAccess::Success && TargetActor != nullptr)
		{
			TSharedPtr<FName>* FindRes = OptionSources.FindByPredicate([TargetDelegateName](TSharedPtr<FName> Option)
			{
				return *Option == TargetDelegateName;
			});

			if (!FindRes)
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
                        FString::Printf(TEXT("Delegate 绑定数据异常 %s"), *Blueprint->GetFullName())));
			}
		}
	}

	FName BindingFuncName;
	FPropertyAccess::Result FuncResult = TriggerFunctionHandle->GetValue(BindingFuncName);
	if (FuncResult == FPropertyAccess::Success && !BindingFuncName.IsNone())
	{
		if (!IsValidBindFunction(Blueprint->SkeletonGeneratedClass, BindingFuncName))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
                    FString::Printf(TEXT("Function 绑定数据异常 %s"), *Blueprint->GetFullName())));
		}
	}
}

void FTargetDelegateCustomization::OnComboBoxSelectedItemChange(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo)
{
	if (Item.IsValid())
	{
		TargetDelegateHandle->SetValue(*Item.Get());
	}
}

TSharedRef<SWidget> FTargetDelegateCustomization::OnGenerateComboBoxItem(TSharedPtr<FName> InItem)
{
	return SNew(STextBlock).Text(FText::FromName(*InItem.Get()));
}

FText FTargetDelegateCustomization::GetItemText() const
{
	FName TargetDelegateName;
	TargetDelegateHandle->GetValue(TargetDelegateName);
	return TargetDelegateName.IsNone() ? LOCTEXT("DefaultOption", "请选择委托") : FText::FromName(TargetDelegateName) ;
}

FMulticastDelegateProperty* FTargetDelegateCustomization::GetTargetDelegateProperty()
{
	FMulticastDelegateProperty* Property = nullptr;

	UObject* TargetObject;
	TargetActorHandle->GetValue(TargetObject);

	FName TargetDelegateName;
	TargetDelegateHandle->GetValue(TargetDelegateName);

	if (TargetObject && !TargetDelegateName.IsNone())
	{
		for (TFieldIterator<FMulticastDelegateProperty> It(TargetObject->GetClass()); It; ++It)
		{
			if (TargetDelegateName == It->GetFName())
			{
				Property = *It;
				break;
			}
		}
	}

	return Property;
}

FText FTargetDelegateCustomization::GetFuncButtonText() const
{
	FName TriggerFunction;
	TriggerFunctionHandle->GetValue(TriggerFunction);
	return TriggerFunction.IsNone() ? LOCTEXT("Bind", "绑定") : FText::FromName(TriggerFunction);
}

void FTargetDelegateCustomization::OnBindingBtnClick()
{
	if (Blueprint)
	{
		if (FMulticastDelegateProperty* Property = GetTargetDelegateProperty())
		{
			UEdGraph* FunctionGraph = FBlueprintEditorUtils::CreateNewGraph(
                Blueprint, 
                FBlueprintEditorUtils::FindUniqueKismetName(Blueprint, TEXT("On_Broadcast")),
                UEdGraph::StaticClass(),
                UEdGraphSchema_K2::StaticClass());
				
			FunctionGraph->bAllowRenaming = false;
	
			FBlueprintEditorUtils::AddFunctionGraph(Blueprint, FunctionGraph, true, Property->SignatureFunction);
			FunctionGraph->Nodes[0]->bCanRenameNode = false;

			TriggerFunctionHandle->SetValue(FunctionGraph->GetFName());

			TSharedPtr<IBlueprintEditor> BlueprintEditor = FKismetEditorUtilities::GetIBlueprintEditorForObject(Blueprint, true);
			BlueprintEditor->OpenGraphAndBringToFront(FunctionGraph);
		}
	}
}

void FTargetDelegateCustomization::OnRemoveBtnClick()
{
	TriggerFunctionHandle->SetValue(FName(NAME_None));
}

void FTargetDelegateCustomization::OnBindingFuncSelected(FName SelectedFuncName)
{
	TriggerFunctionHandle->SetValue(SelectedFuncName);
}

TSharedRef<SWidget> FTargetDelegateCustomization::OnGenerateDelegateMenu()
{
	FMulticastDelegateProperty* DelegateProperty = GetTargetDelegateProperty();
	if (DelegateProperty == nullptr)
	{
		return SNullWidget::NullWidget;
	}
	
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("BindingActions", LOCTEXT("Operations", "操作"));

	FName TriggerFunction;
	TriggerFunctionHandle->GetValue(TriggerFunction);

	if ( TriggerFunction.IsNone() )
	{
		MenuBuilder.AddMenuEntry(
            LOCTEXT("CreateBinding", "创建绑定"),
            LOCTEXT("CreateBindingToolTip", "Create Binding"),
            FSlateIcon(FEditorStyle::GetStyleSetName(), "Plus"),
            FUIAction(FExecuteAction::CreateSP(this, &FTargetDelegateCustomization::OnBindingBtnClick))
            );
	}
	else
	{
		MenuBuilder.AddMenuEntry(
            LOCTEXT("RemoveBinding", "移除绑定"),
            LOCTEXT("RemoveBindingToolTip", "Removes the current binding"),
            FSlateIcon(FEditorStyle::GetStyleSetName(), "Cross"),
            FUIAction(FExecuteAction::CreateSP(this, &FTargetDelegateCustomization::OnRemoveBtnClick))
            );
	}

	MenuBuilder.EndSection(); //CreateBinding
	
	MenuBuilder.BeginSection("SelectedFunctions", LOCTEXT("Functions", "绑定函数"));

	TArray<FParamInformation> Params;
	for( TFieldIterator<FProperty> It(DelegateProperty->SignatureFunction); It && (It->PropertyFlags & CPF_Parm); ++It )
	{
		FParamInformation Information;
		Information.Property = *It;
		Information.bIsReturnParam = (It->PropertyFlags & CPF_ReturnParm) > 0;
		Params.Add(Information);
	}
	
	for( TFieldIterator<UFunction> It(Blueprint->SkeletonGeneratedClass, EFieldIteratorFlags::ExcludeSuper); It; ++It )
	{
		if (It->NumParms != Params.Num() || (It->FunctionFlags & FUNC_Event))
		{
			continue;
		}

		int Index = 0;
		bool bMatch = true;
		for( TFieldIterator<FProperty> PIt(*It); PIt && (PIt->PropertyFlags & CPF_Parm); ++PIt, Index++ )
		{
			bMatch &= (Params[Index].Property->GetCPPType() == PIt->GetCPPType() && !(Params[Index].bIsReturnParam ^ ((PIt->PropertyFlags & CPF_ReturnParm) > 0)));
			//UE_LOG(LogTemp, Error, TEXT("%s <> %s"), *Params[Index].Property->GetCPPType(), *PIt->GetCPPType());
		}

		if (bMatch)
		{
			FName MatchFuncName = It->GetFName();
			MenuBuilder.AddMenuEntry(
                FText::FromName(MatchFuncName),
                LOCTEXT("SelectedFunc", "选择已有的函数"),
                FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.K2Node_FunctionEntry"),
                FUIAction(FExecuteAction::CreateSP(this, &FTargetDelegateCustomization::OnBindingFuncSelected, MatchFuncName))
                );
		}
	}

	MenuBuilder.EndSection();

	return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
            MenuBuilder.MakeWidget()
        ];
}

bool FTargetDelegateCustomization::IsValidBindFunction(UClass* Class, const FName& FuncName)
{
	bool bValid = false;
	UFunction* BindFunction = nullptr;
	
	for( TFieldIterator<UFunction> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It )
	{
		if (It->FunctionFlags & (FUNC_Event))
		{
			continue;
		}

		if (FuncName == It->GetFName())
		{
			bValid = true;
			BindFunction = *It;
			break;
		}
	}

	FMulticastDelegateProperty* Property = GetTargetDelegateProperty();
	if (bValid && Property && BindFunction)
	{
		if (Property->SignatureFunction->NumParms == BindFunction->NumParms)
		{
			TArray<FParamInformation> Params;
			for( TFieldIterator<FProperty> It(Property->SignatureFunction); It && (It->PropertyFlags & CPF_Parm); ++It )
			{
				FParamInformation Information;
				Information.Property = *It;
				Information.bIsReturnParam = (It->PropertyFlags & CPF_ReturnParm) > 0;
				Params.Add(Information);
			}

			int N = 0;
			for( TFieldIterator<FProperty> PIt(BindFunction); PIt && (PIt->PropertyFlags & CPF_Parm); ++PIt, N++ )
			{
				bValid &= (Params[N].Property->GetCPPType() == PIt->GetCPPType() &&
                    !(Params[N].bIsReturnParam ^ ((PIt->PropertyFlags & CPF_ReturnParm) > 0)));
			}
		}
		else
		{
			bValid = false;
		}
	}
	
	return bValid;
}

bool FTargetDelegateCustomization::IsTargetDelegateModify()
{
	FName TargetDelegate;
	TargetDelegateHandle->GetValue(TargetDelegate);
	return !TargetDelegate.IsNone();
}

bool FTargetDelegateCustomization::IsTriggerFunctionModify()
{
	FName TriggerFunction;
	TriggerFunctionHandle->GetValue(TriggerFunction);
	return !TriggerFunction.IsNone();
}

void FTargetDelegateCustomization::OnTargetActorChanged()
{
	RefreshData();
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
        LOCTEXT("MenuLabel1", "添加到预览网格体(Active)"),
        LOCTEXT("MenuLabel1Tooltips", "附加到手中"),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateSP(this, &FWeaponMeshPreviewCustomizationMenu::OnExecuteAction),
            FCanExecuteAction::CreateSP(this, &FWeaponMeshPreviewCustomizationMenu::OnCanExecuteAction)
        )
    );

	MenuBuilder.AddMenuEntry(
        LOCTEXT("MenuLabel2", "添加到预览网格体(Holster)"),
        LOCTEXT("MenuLabel2Tooltips", "附加到背后"),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateSP(this, &FWeaponMeshPreviewCustomizationMenu::OnExecuteAction2),
            FCanExecuteAction::CreateSP(this, &FWeaponMeshPreviewCustomizationMenu::OnCanExecuteAction)
        )
    );
}

void FWeaponMeshPreviewCustomizationMenu::OnExecuteAction() const
{
	AShootWeapon* WeaponCDO = SelectedWeaponClass->GetDefaultObject<AShootWeapon>();

	if (USkeleton* Skeleton = UDMDevelopmentSettings::Get()->GetPreviewTargetSkeletal())
	{
		FName PreviewSocketName = UDMDevelopmentSettings::Get()->GetActivePreviewSocketName();
		
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

void FWeaponMeshPreviewCustomizationMenu::OnExecuteAction2() const
{
	AShootWeapon* WeaponCDO = SelectedWeaponClass->GetDefaultObject<AShootWeapon>();

	if (USkeleton* Skeleton = UDMDevelopmentSettings::Get()->GetPreviewTargetSkeletal())
	{
		FName PreviewSocketName = UDMDevelopmentSettings::Get()->GetHolsterPreviewSocketName();
		
		USkeletalMeshSocket* PreviewSocket = Skeleton->FindSocket(PreviewSocketName);

		PreviewSocket->RelativeLocation = WeaponCDO->WeaponHolsterSocketOffset.GetLocation();
		PreviewSocket->RelativeRotation = WeaponCDO->WeaponHolsterSocketOffset.Rotator();
		PreviewSocket->RelativeScale = WeaponCDO->WeaponHolsterSocketOffset.GetScale3D();
		
		Skeleton->PreviewAttachedAssetContainer.ClearAllAttachedObjects();
		Skeleton->PreviewAttachedAssetContainer.AddAttachedObject(WeaponCDO->WeaponMesh->SkeletalMesh, PreviewSocketName);
		Skeleton->Modify();
	}
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
#undef LOCTEXT_NAMESPACE