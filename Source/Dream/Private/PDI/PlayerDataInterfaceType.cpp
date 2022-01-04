// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable CppMemberFunctionMayBeConst
#include "PDI/PlayerDataInterfaceType.h"
#include "DEquipmentPerkPool.h"
#include "DGameplayStatics.h"
#include "DreamGameInstance.h"
#include "DreamWidgetStatics.h"

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

void FQuestActionContainer::SerializeData(FArchive& Ar)
{
	FQuestActionBase::SerializeData(Ar);
	
	if (Ar.IsSaving())
	{
		int32 ActionNum = Actions.Num();
		Ar << ActionNum;

		for (TSharedPtr<FQuestActionBase> Action : Actions)
		{
			Action->SerializeData(Ar);
		}
	}
}

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
	case ETaskCondGUID::CONTAINER:
		Condition = NewObject<UDQuestCondition_Container>();
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

float UDQuestCondition_Container::GetQuestProgressPercent() const
{
	int32 Num = Conditions.Num();
	if (Num == 0)
	{
		return 0.f;
	}

	float PercentAvg = 0.f;
	for (UDQuestCondition* Condition : Conditions)
	{
		check(Condition);
		PercentAvg += Condition->GetQuestProgressPercent();
	}

	return PercentAvg / Num;
}

void UDQuestCondition_Container::SerializeData(FArchive& Ar)
{
	Super::SerializeData(Ar);

	int32 ConditionNum = Conditions.Num();
	Ar << ConditionNum;
	
	if (Ar.IsLoading())
	{
		Conditions.SetNumZeroed(ConditionNum);
		for (int32 N = 0; N < ConditionNum; N++)
		{
			Conditions[N] = UDQuestCondition::Decode(Ar);
		}
	}
	else
	{
		for (int32 N = 0; N < ConditionNum; N++)
		{
			Conditions[N]->SerializeData(Ar);
		}
	}
}

#if WITH_EDITOR

void UDQuestCondition_Container::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property == nullptr)
	{
		return;
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UDQuestCondition_Container, Conditions))
	{
		for (TArray<UDQuestCondition*>::TIterator Iterator = Conditions.CreateIterator(); Iterator; ++Iterator)
		{
			UDQuestCondition* Current = *Iterator;
			if (Current && Current->IsA<UDQuestCondition_Container>())
			{
				Iterator.RemoveCurrent();
			}
		}
	}
}
#endif

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

void UItemDataEquipment::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);

	Ar << Attributes;
}

UItemDataEquipment* UItemDataEquipment_Random::CastToEquipment()
{
	if (AttrPool)
	{
		UItemDataEquipment* Equipment = NewObject<UItemDataEquipment>();
		AttrPool->GenerateAttributes(Equipment->Attributes);

		return Equipment;
	}
	
	return nullptr;
}

void UItemDataNumericalValue::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);

	Ar << Value;
}

void UItemData::SerializeItemData(FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		Ar << ItemGuid;
	}
}

UItemData* UItemData::Decode(FArchive& Ar)
{
	check(Ar.IsLoading());

	int32 Guid;
	Ar << Guid;

	int32 ItemType = ::GetItemType(Guid);

	UItemData* ItemData;
	
	switch (ItemType)
	{
		case EItemType::Weapon:
		case EItemType::Module:
			ItemData = NewObject<UItemDataEquipment>();
			break;
		case EItemType::Experience:
		case EItemType::Material:
			ItemData = NewObject<UItemDataNumericalValue>();
			break;
		default:
			ItemData = nullptr;
	}

	if (ItemData != nullptr)
	{
		ItemData->ItemGuid = Guid;
		ItemData->SerializeItemData(Ar);
	}

	return ItemData;
}

UClass* UItemData::GetItemClass() const
{
	return UDreamWidgetStatics::GetItemClassByGuid(ItemGuid);
}

const FPropsInfo& UItemData::GetPropsInfo() const
{
	return UDreamWidgetStatics::GetPropsInfoByItemGuid(ItemGuid);
}

TEnumAsByte<EItemType::Type> UItemData::GetItemType() const
{
	return ::GetItemType(ItemGuid);
}

UItemData* FItemDataHandle::Get() const
{
	return ItemData;
}

FString FSearchServerResult::GetConnectURL(int32 PlayerID) const
{
	return FString::Printf(TEXT("%s?PlayerId=%d"), *ServerAddress, PlayerID);
}

#undef LOCTEXT_NAMESPACE
