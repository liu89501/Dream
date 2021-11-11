
#include "CoreMinimal.h"


#include "DEnemyShooter.h"
#include "DMoney.h"
#include "Misc/AutomationTest.h"
#include "DreamType.h"
#include "JsonObjectConverter.h"
#include "PlayerDataInterfaceBase.h"
#include "PlayerDataInterfaceType.h"
#include "PlayerGameData.h"
#include "ShootWeapon.h"
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

	DREAM_NLOG(Log, TEXT("----------- <><><><><><><> -------------"));

	TMap<FString, int32> Result2;
	for (int32 i = 0; i < TestCount; i++)
	{
		if (UKismetMathLibrary::RandomBoolWithWeight(0.01))
		{
			Result2.FindOrAdd(TEXT("百分之1"))++;
		}

		if (UKismetMathLibrary::RandomBoolWithWeight(0.2))
		{
			Result2.FindOrAdd(TEXT("百分之20"))++;
		}
		
		if (UKismetMathLibrary::RandomBoolWithWeight(0.1))
		{
			Result2.FindOrAdd(TEXT("百分之10"))++;
		}
	}

	for (TPair<FString, int32> Pair : Result2)
	{
		DREAM_NLOG(Log, TEXT("Name: %s, Result: %f"), *Pair.Key, (float)Pair.Value / TestCount);
	}

	DREAM_NLOG(Log, TEXT("----------- 随机工具测试 结束 -------------"));

	DREAM_NLOG(Log, TEXT("----------- 其他测试 开始 -------------"));

	/*UTaskData* LoadGameFromSlot = Cast<UTaskData>(UGameplayStatics::LoadGameFromSlot(TEXT("TestSaveGame"), 0));

	if (LoadGameFromSlot)
	{
		UDQuestCondition_KillTarget* KillTarget = Cast<UDQuestCondition_KillTarget>(LoadGameFromSlot->TaskList[0].CompleteCondition.Get());
		DREAM_NLOG(Log, TEXT("Load CurrentKilled: %d"), KillTarget->CurrentKilled);
		DREAM_NLOG(Log, TEXT("Load KillNum: %d"), KillTarget->KillNum);
		DREAM_NLOG(Log, TEXT("Load TargetType: %s"), *KillTarget->TargetType->GetPathName());

		KillTarget->CurrentKilled++;

		UGameplayStatics::SaveGameToSlot(LoadGameFromSlot, TEXT("TestSaveGame"), 0);
	}
	else
	{
		UTaskData* TaskDataTest = NewObject<UTaskData>();
		FTaskInformationSaveGame Information;
		Information.TaskId = 10;
		Information.TaskMark = ETaskMark::Completed;

		Information.CompletedReward = UItemDataMoney::StaticClass()->GetDefaultObject<UItemDataMoney>();

		UDQuestCondition_KillTarget* KillTarget = NewObject<UDQuestCondition_KillTarget>();
		KillTarget->KillNum = 10;
		KillTarget->CurrentKilled = 1;
		KillTarget->TargetType = ADEnemyShooter::StaticClass();

		Information.CompleteCondition = FQuestConditionHandle(KillTarget);

		TaskDataTest->TaskList.Add(Information);

		UGameplayStatics::SaveGameToSlot(TaskDataTest, TEXT("TestSaveGame"), 0);

		DREAM_NLOG(Log, TEXT("Save TaskDataTest: %s"), *TaskDataTest->GetFullName());
	}*/

	

	FLoginParameter Parameter;
	Parameter.PlatformName = TEXT("Steam");
	Parameter.ThirdPartyUserTicket = TEXT("AA");

	FMyTestSerialize Test;
	Test.IntArr = 12.3f;
	FSoftObjectPath Path(TEXT("/Game/Main/Weapon/PrecisionRifle/Jotunn"));

	TSet<int32> S;
	S.Add(1);
	S.Add(2);
	Test.SetTest = S;
	Test.ArrSer.Add(Parameter);

	TestSerialize(Parameter);
	TestSerialize(Test);

	/*TArray<uint8> AA(RawData, 645);
	
	FPacketArchiveReader Reader(AA);

	FPlayerInfo Info;
	Reader << Info;

	DREAM_NLOG(Error, TEXT("MaxExperience %d"), Info.Properties.MaxExperience);*/
	
	DREAM_NLOG(Log, TEXT("----------- 其他测试 结束 -------------"));

	return true;
}


