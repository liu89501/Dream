
#include "CoreMinimal.h"


#include "DEnemyShooter.h"
#include "DMoney.h"
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

template<typename Type>
void TestSerialize(Type Parameter)
{
	TArray<uint8> Data;
	FPacketArchiveWriter Writer(Data);
	int32 Mark = 20;
	Writer << Mark;
	Writer << Parameter;
	TArray<uint8> Packet;
	int32 DataLength = Data.Num();
	Packet.Append((uint8*)&DataLength, sizeof(int32));
	Packet.Append(Data);
	
	FString Bytes;
	for (uint8 b : Packet)
	{
		Bytes.AppendInt(b);
		Bytes.Append(TEXT(","));
	}

	UE_LOG(LogDream, Log, TEXT("Bytes: %s"), *Bytes);
}

bool FDreamTests::RunTest(const FString& Parameters) 
{

	DREAM_NLOG(Log, TEXT("----------- Json相关测试 开始 -------------"));

	TSharedPtr<FJsonObject> Object1 = TSharedPtr<FJsonObject>(new FJsonObject);
	TSharedPtr<FJsonObject> Object2 = TSharedPtr<FJsonObject>(new FJsonObject);
	TSharedRef< TJsonReader<TCHAR> > Reader1 = FJsonStringReader::Create(TEXT("{")
        TEXT("\"itemId\": 10,")
        TEXT("\"type\": \"Weapon\",")
        TEXT("\"itemClass\": \"/Game/Main/Weapon/PrecisionRifle/Jotunn.Jotunn_C\",")
        TEXT("\"itemPrice\": 5000")
    TEXT("}"));
	TSharedRef< TJsonReader<TCHAR> > Reader2 = FJsonStringReader::Create(TEXT("{")
        TEXT("\"itemId\": 10,")
        TEXT("\"type\": 1,")
        TEXT("\"itemClass\": \"/Game/Main/Weapon/PrecisionRifle/Jotunn.Jotunn_C\",")
        TEXT("\"itemPrice\": 5000")
    TEXT("}"));

	FJsonSerializer::Deserialize(Reader1, Object1);
	FJsonSerializer::Deserialize(Reader2, Object2);

	DREAM_NLOG(Log, TEXT("----------- Json相关测试 开始 -------------"));
	
	
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

	int32 TestCount = 1000;

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

	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
		if (ActorCDO && ActorCDO->GetIsReplicated())
		{
			DREAM_NLOG(Log, TEXT("Class: %s"), *Class->GetFullName());
		}
	}

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

	DREAM_NLOG(Log, TEXT("----------- 其他测试 结束 -------------"));
	

	return true;
}


