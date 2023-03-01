// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable CppMemberFunctionMayBeConst
#include "PDI/PlayerDataInterfaceType.h"
#include "DEquipmentPerkPool.h"
#include "DGameplayStatics.h"

#define LOCTEXT_NAMESPACE "PlayerDataType"

/*FArchive& operator<<(FArchive& Ar, UClass*& Class)
{
	if (Ar.IsLoading())
	{
		FString ClassPath;
		Ar << ClassPath;

		
	}
	else
	{
		FString ClassPath = Class->GetPathName();
		Ar << ClassPath;

		
	}
}*/

void FQuestAction_KilledTarget::SerializeData(FArchive& Ar)
{
	FQuestActionBase::SerializeData(Ar);

	Ar << TargetPawn;
}

void FQuestAction_Event::SerializeData(FArchive& Ar)
{
	FQuestActionBase::SerializeData(Ar);
	Ar << EventName;
}

float UDQuestCondition::BP_GetQuestProgressPercent() const
{
	return GetQuestProgressPercent();
}

FText UDQuestCondition::BP_GetConditionDesc() const
{
	return NativeGetConditionDesc();
}

bool UDQuestCondition::IsCompleted() const
{
	return bCompleted;
}

FText UDQuestCondition::NativeGetConditionDesc() const
{
	return FText::FromString(TEXT(""));
}

float UDQuestCondition::GetQuestProgressPercent() const
{
	return 0;
}

void UDQuestCondition::SerializeData(FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		Ar << ConditionGUID; 
	}

	Ar << bCompleted;
}

UDQuestCondition* UDQuestCondition::Decode(FArchive& Ar)
{
	check(Ar.IsLoading());
	
	ETaskCondGUID ConditionGUID;
	Ar << ConditionGUID;

	UDQuestCondition* Condition;

	switch (ConditionGUID)
	{
	case ETaskCondGUID::KILL_TARGET:
		Condition = NewObject<UDQuestCondition_KillTarget>();
		break;
	case ETaskCondGUID::EVENT:
		Condition = NewObject<UDQuestCondition_Event>();
		break;
	default:
		Condition = nullptr;
	}

	if (Condition != nullptr)
	{
		Condition->ConditionGUID = ConditionGUID;
		Condition->SerializeData(Ar);
	}

	return Condition;
}

float UDQuestCondition_Count::GetQuestProgressPercent() const
{
	return TargetValue == 0 ? 0 : CurrentValue / static_cast<float>(TargetValue);
}

void UDQuestCondition_Count::SerializeData(FArchive& Ar)
{
	Super::SerializeData(Ar);

	Ar << CurrentValue;
	Ar << TargetValue;
}

FText UDQuestCondition_KillTarget::NativeGetConditionDesc() const
{
	static FText Desc = LOCTEXT("COND_KillTarget", "击杀目标");
	return Desc;
}

void UDQuestCondition_KillTarget::SerializeData(FArchive& Ar)
{
	Super::SerializeData(Ar);
	
	Ar << TargetType;
}

float UDQuestCondition_Event::GetQuestProgressPercent() const
{
	return bCompleted ? 1.f : 0.f;
}

FText UDQuestCondition_Event::NativeGetConditionDesc() const
{
	static FText Desc = LOCTEXT("COND_Event", "完成事件");
    return Desc;
}

void UDQuestCondition_Event::SerializeData(FArchive& Ar)
{
	Super::SerializeData(Ar);

	Ar << EventName;
}

FArchive& operator<<(FArchive& Ar, FQuestConditionHandle& Other)
{
	int32 ConditionNum = Other.Conditions.Num();
	Ar << ConditionNum;
	
	if (Ar.IsLoading())
	{
		Other.Conditions.SetNumZeroed(ConditionNum);
		for (int32 N = 0; N < ConditionNum; N++)
		{
			Other.Conditions[N] = UDQuestCondition::Decode(Ar);
		}
	}
	else
	{
		for (int32 N = 0; N < ConditionNum; N++)
		{
			Other.Conditions[N]->SerializeData(Ar);
		}
	}
	
	return Ar;
}

FArchive& operator<<(FArchive& Ar, FTaskInformation& A)
{
	Ar << A.TaskId;
	Ar << A.TaskGroupId;
	Ar << A.TaskDescription;
	Ar << A.TaskMark;
	Ar << A.bTracking;
	Ar << A.Condition;
	Ar << A.CompletedReward;
	return Ar;
}

void FItem::Serialize(FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		Ar << ItemGuid;
	}
}

void FItemEquipment::Serialize(FArchive& Ar)
{
	FItem::Serialize(Ar);

	Ar << Attributes;
	Ar << GearLevel;
}

void FItemSimple::Serialize(FArchive& Ar)
{
	FItem::Serialize(Ar);

	Ar << ItemNum;
}

TSharedPtr<FItem> FItemHelper::Decode(FArchive& Ar)
{
	int32 ItemGuid = 0;
	Ar << ItemGuid;

	TSharedPtr<FItem> Item;

	EItemType::Type ItemType = ItemUtils::GetItemType(ItemGuid);

	switch (ItemType)
	{
	case EItemType::Weapon:
    case EItemType::Module:
		
        Item = MakeShared<FItemEquipment>(ItemGuid);
		break;
		
	default:
		Item = MakeShared<FItemSimple>(ItemGuid);
	}

	Item->Serialize(Ar);

	return Item;
}

TSharedPtr<FItem> UItemDataEquipment::MakeItemStruct()
{
	return MakeShared<FItemEquipment>(GuidHandle.ItemGuid, Level,Attributes);
}

TSharedPtr<FItem> UItemDataEquipment_Random::MakeItemStruct()
{
	if (AttrPool)
	{
		FEquipmentAttributes Attributes;
		AttrPool->GenerateAttributes(Attributes);
		return MakeShared<FItemEquipment>(GuidHandle.ItemGuid, LevelRange.GetRandomInt(), Attributes);
	}
	
	return nullptr;
}

TSharedPtr<FItem> UItemDataNumericalValue::MakeItemStruct()
{
	return MakeShared<FItemSimple>(GuidHandle.ItemGuid, Value);
}

TSharedPtr<FItem> UItemDataNumericalValue_Random::MakeItemStruct()
{
	return MakeShared<FItemSimple>(GuidHandle.ItemGuid, RangeValue.GetRandomInt());
}

FString FLaunchServerResult::GetConnectURL(int32 PlayerID) const
{
	return FString::Printf(TEXT("%s?PlayerId=%d"), *ServerAddress, PlayerID);
}

FEmptyParam FEmptyParam::SINGLETON = FEmptyParam();

#undef LOCTEXT_NAMESPACE
