
#include "CoreMinimal.h"
#include "DEnemyShooter.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/AutomationTest.h"
#include "DreamType.h"
#include "JsonObjectConverter.h"
#include "PlayerDataInterfaceBase.h"
#include "PlayerDataInterfaceType.h"
#include "PlayerGameData.h"
#include "ShootWeapon.h"
#include "SocketSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDreamTests, "Dream.Default", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)

bool FDreamTests::RunTest(const FString& Parameters) 
{
	DREAM_NLOG(Log, TEXT("----------- 去重迭代器测试 开始 -------------"));
	TArray<int32> TestArr;
	TestArr.Add(1);
	TestArr.Add(1);
	TestArr.Add(1);
	TestArr.Add(2);
	TestArr.Add(3);
	TestArr.Add(3);
	TestArr.Add(2);
	TestArr.Add(22);
	TestArr.Add(23);

	for (FArrayDistinctIterator<int32> AD(TestArr); AD; ++AD)
	{
		DREAM_NLOG(Log, TEXT("Result: %d"), *AD);
	}
	
	DREAM_NLOG(Log, TEXT("----------- 去重迭代器测试 结束 -------------"));
	
	
	DREAM_NLOG(Log, TEXT("----------- 随机工具测试 开始 -------------"));
	TMap<FString, float> Source;
	Source.Add(TEXT("元素5"), 1.f);
	Source.Add(TEXT("元素6"), 0.1f);
	Source.Add(TEXT("元素1"), 20.f);
	Source.Add(TEXT("元素2"), 30.f);
	Source.Add(TEXT("元素3"), 10.f);
	Source.Add(TEXT("元素4"), 20.f);

	TMap<FString, int32> Result;

	int32 TestCount = 100;

	for (int32 i = 0; i < TestCount; i++)
	{
		FString WinningResult = FRandomProbability::RandomProbabilityEx(Source);

		if (WinningResult.IsEmpty())
		{
			int32& Count = Result.FindOrAdd(TEXT("未抽中"));
			Count++;
		}
		else
		{
			int32& Count = Result.FindOrAdd(WinningResult);
			Count++;
		}
	}

	for (TPair<FString, int32> Pair : Result)
	{
		DREAM_NLOG(Log, TEXT("Name: %s, Result: %f"), *Pair.Key, (float)Pair.Value / TestCount);
	}

	DREAM_NLOG(Log, TEXT("----------- 随机工具测试 结束 -------------"));

	DREAM_NLOG(Log, TEXT("----------- 其他测试 开始 -------------"));

	/*for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
		if (ActorCDO && ActorCDO->GetIsReplicated())
		{
			DREAM_NLOG(Log, TEXT("Class: %s"), *Class->GetFullName());
		}
	}*/

	/*FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	UClass* BaseClass = UDModuleBase::StaticClass();

	TSet<FString> NativeChildClasses;
	
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(BaseClass))
		{
			FString PathName = It->GetPathName();

			NativeChildClasses.Add(PathName);
		}
	}

	TArray<FAssetData> Assets;
	
	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(TEXT("/Game/Main/Module"));
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());

	AssetRegistry.GetAssets(Filter, Assets);

	for (const FAssetData& AssetData : Assets)
	{
		uint32 AssetClassFlags;
		AssetData.GetTagValue<uint32>(FBlueprintTags::ClassFlags, AssetClassFlags);

		if (AssetClassFlags & EClassFlags::CLASS_Abstract)
		{
			continue;
		}

		FString NativeParentClassPath;
		AssetData.GetTagValue(FBlueprintTags::NativeParentClassPath, NativeParentClassPath);

		FString ImplementedInterfaces;
		AssetData.GetTagValue(FBlueprintTags::ImplementedInterfaces, ImplementedInterfaces);
		DREAM_NLOG(Log, TEXT("ImplementedInterfaces: %s"), *ImplementedInterfaces);

		FString NativeClassPath = FPackageName::ExportTextPathToObjectPath(NativeParentClassPath);

		if (NativeChildClasses.Contains(NativeClassPath))
		{
			FString ClassObjectPath;
			AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, ClassObjectPath);

			FString WeaponBlueprintClass = FPackageName::ExportTextPathToObjectPath(ClassObjectPath);

			UClass* Class = LoadClass<UObject>(nullptr, *WeaponBlueprintClass);

			if (IPropsInterface* PropsInterface = Cast<IPropsInterface>(Class->GetDefaultObject()))
			{
				DREAM_NLOG(Log, TEXT("Weapon Class: %s   Name: %s"), *WeaponBlueprintClass, *PropsInterface->GetPropsInfo().PropsName.ToString());
			}
		}
	}*/

	/*UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Main/Asset/DT_Items"));

	if (DataTable != nullptr)
	{
		const TMap<FName, uint8*> RowMap = DataTable->GetRowMap();

		for (const TTuple<FName, uint8*>& Row : RowMap)
		{
			if (FItemDefinition* Def = reinterpret_cast<FItemDefinition*>(Row.Value))
			{
				if (UClass* Class = LoadClass<UObject>(nullptr, *Def->ItemClass))
				{
					if (Class->ImplementsInterface(IPropsInterface::UClassType::StaticClass()))
					{
						if (IPropsInterface* PropsInterface = Cast<IPropsInterface>(Class->GetDefaultObject()))
						{
							const FPropsInfo& PropsInfo = PropsInterface->GetPropsInfo();

							UE_LOG(LogDream, Log, TEXT("%s,%d"), *Row.Key.ToString(), PropsInfo.PropsQuality);
						}
					}
				}
			}
		}
	}*/

	FAmmunitionDropProbability DropProbability;

	int32 L1 = 0, L2 = 0, L3 = 0, None = 0;
	
	for (int32 Idx = 0; Idx < 100; Idx++)
	{
		EAmmoType AmmoType;
		if (DropProbability.RandomDrawing(AmmoType))
		{
			int32& Res = AmmoType == EAmmoType::Level1 ? L1 : AmmoType == EAmmoType::Level2 ? L2 : L3;
			Res++;
		}
		else
		{
			None++;
		}
	}

	DREAM_NLOG(Log, TEXT("L1: %f, L2: %f, L3: %f, None: %f"), L1 / 100.f, L2 / 100.f, L3 / 100.f, None / 100.f);

	DREAM_NLOG(Log, TEXT("----------- 其他测试 结束 -------------"));
	

	return true;
}


