
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "DreamType.h"
#include "JsonObjectConverter.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/KismetMathLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDreamTests, "Dream.Default", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)

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

	DREAM_NLOG(Log, TEXT("----------- 随机数测试 开始 -------------"));

	for (int32 i = 0; i < 10; i++)
	{
		DREAM_NLOG(Log, TEXT("Number: %lld"), UKismetMathLibrary::RandomInteger64(0xFFFFFFFFFFFFF));
	}

	DREAM_NLOG(Log, TEXT("----------- 随机数测试 结束 -------------"));

	return true;
}
