
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "DreamType.h"
#include "JsonObjectConverter.h"
#include "PlayerDataStoreType.h"
#include "Kismet/KismetMathLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDreamTests, "Dream.Default", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)

bool FDreamTests::RunTest(const FString& Parameters) 
{

	DREAM_NLOG(Log, TEXT("----------- Json相关测试 开始 -------------"));
	FStoreItem Item;
	Item.Type = EItemType::Weapon;
	Item.ItemClass = TEXT("aaaa");
	Item.ItemId = 10;
	Item.ItemPrice = 5000;

	FString OutString;
	FJsonObjectConverter::UStructToJsonObjectString<FStoreItem>(Item, OutString);
	DREAM_NLOG(Log, TEXT("Serialize: %s"), *OutString);

	TSharedPtr<FJsonObject> Object1 = TSharedPtr<FJsonObject>(new FJsonObject);
	TSharedPtr<FJsonObject> Object2 = TSharedPtr<FJsonObject>(new FJsonObject);
	TSharedRef< TJsonReader<TCHAR> > Reader1 = FJsonStringReader::Create(TEXT("{")
        TEXT("\"itemId\": 10,")
        TEXT("\"type\": \"Weapon\",")
        TEXT("\"itemClass\": \"aaaa\",")
        TEXT("\"itemPrice\": 5000")
    TEXT("}"));
	TSharedRef< TJsonReader<TCHAR> > Reader2 = FJsonStringReader::Create(TEXT("{")
        TEXT("\"itemId\": 10,")
        TEXT("\"type\": 1,")
        TEXT("\"itemClass\": \"aaaa\",")
        TEXT("\"itemPrice\": 5000")
    TEXT("}"));

	FJsonSerializer::Deserialize(Reader1, Object1);
	FJsonSerializer::Deserialize(Reader2, Object2);

	FStoreItem OutItem1;
	FStoreItem OutItem2;
	FJsonObjectConverter::JsonObjectToUStruct(Object1.ToSharedRef(), &OutItem1);
	FJsonObjectConverter::JsonObjectToUStruct(Object2.ToSharedRef(), &OutItem2);
	
	DREAM_NLOG(Log, TEXT("Deserialize, type: %s"), *UEnum::GetValueAsString(OutItem1.Type));
	DREAM_NLOG(Log, TEXT("Deserialize, itemClass: %s"), *OutItem1.ItemClass);
	DREAM_NLOG(Log, TEXT("Deserialize, itemPrice: %lld"), OutItem1.ItemPrice);
	
	DREAM_NLOG(Log, TEXT("Deserialize, type: %s"), *UEnum::GetValueAsString(OutItem2.Type));
	DREAM_NLOG(Log, TEXT("Deserialize, itemClass: %s"), *OutItem2.ItemClass);
	DREAM_NLOG(Log, TEXT("Deserialize, itemPrice: %lld"), OutItem2.ItemPrice);


	FPlayerWeapon Weapon;
	Weapon.EquipmentIndex = 1;
	Weapon.WeaponClass = TEXT("/GAME/AAA");
	Weapon.WeaponId = 5;
	
	FPlayerInfo PlayerInfo;
	PlayerInfo.Money = 10000;
	PlayerInfo.MaxWeaponNum = 50;
	PlayerInfo.Weapons.Add(Weapon);

	FString PlayerOut;
	FJsonObjectConverter::UStructToJsonObjectString(PlayerInfo, PlayerOut);
	DREAM_NLOG(Log, TEXT("PlayerInfo: %s"), *PlayerOut);

	TSharedRef< TJsonReader<TCHAR> > Reader3 = FJsonStringReader::Create(PlayerOut);

	TSharedPtr<FJsonObject> PlayerObj = MakeShareable(new FJsonObject);
	
	FJsonSerializer::Deserialize(Reader3, PlayerObj);

	FPlayerInfo PlayerInfoNew;
	FJsonObjectConverter::JsonObjectToUStruct(PlayerObj.ToSharedRef(), &PlayerInfoNew);
	DREAM_NLOG(Log, TEXT("PlayerInfo Deserialize, maxWeaponNum:%d, Money: %lld, Weapons: %d"),
		PlayerInfoNew.MaxWeaponNum, PlayerInfoNew.Money, PlayerInfoNew.Weapons.Num());
	

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
		FString WinningResult = FRandomProbability::RandomProbability(Source);

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

	return true;
}
