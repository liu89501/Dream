// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable CppMemberFunctionMayBeConst
#include "PDI/PlayerDataInterfaceType.h"
#include "DExperience.h"
#include "DGameplayStatics.h"
#include "DModuleBase.h"
#include "DMoney.h"
#include "ShootWeapon.h"

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

float UDQuestCondition::BP_GetQuestProgressPercent() const
{
	return GetQuestProgressPercent();
}

float UDQuestCondition::GetQuestProgressPercent() const
{
	return 0.f;
}

bool UDQuestCondition::UpdateCondition(const FQuestActionHandle& Handle)
{
	return false;
}

bool UDQuestCondition::IsCompleted() const
{
	return false;
}

void UDQuestCondition::SerializeItemData(FArchive& Ar)
{
}

bool UDQuestCondition_KillTarget::UpdateCondition(const FQuestActionHandle& Handle)
{
	if (IsCompleted())
	{
		return true;
	}
	
	if (FQuestAction* QuestAction = Handle.GetData())
	{
		if (QuestAction->GetStructType() == FQuestAction_KilledTarget::StaticStruct())
		{
			FQuestAction_KilledTarget* Action_KilledTarget = static_cast<FQuestAction_KilledTarget*>(QuestAction);
			if (Action_KilledTarget->GetTargetClass()->IsChildOf(*TargetType))
			{
				FPlatformAtomics::InterlockedIncrement(&CurrentKilled);
			}
		}
	}

	return IsCompleted();
}

float UDQuestCondition_KillTarget::GetQuestProgressPercent() const
{
	return CurrentKilled / static_cast<float>(KillNum);
}

bool UDQuestCondition_KillTarget::IsCompleted() const
{
	return FMath::Min(CurrentKilled, KillNum) == KillNum;
}

void UDQuestCondition_KillTarget::SerializeItemData(FArchive& Ar)
{
	Ar << CurrentKilled;
	Ar << KillNum;
	Ar << TargetType;
}

UClass* UItemData::GetItemClass() const
{
	return nullptr;
}

int32 UItemData::GetItemAmount() const
{
	return 1;
}

EItemType::Type UItemData::NativeGetItemType() const
{
	return EItemType::INVALID;
}

bool UItemData::IsValidData() const
{
	return true;
}

const FPropsInfo& UItemData::GetPropsInformation() const
{
	if (UClass* ItemClass = GetItemClass())
	{
		if (IPropsInterface* PropsInterface = Cast<IPropsInterface>(ItemClass->GetDefaultObject()))
		{
			return PropsInterface->GetPropsInfo();
		}
	}

	return FEmptyStruct::EmptyPropsInfo;
}

TEnumAsByte<EItemType::Type> UItemData::GetItemType() const
{
	return NativeGetItemType();
}

void UItemDataContainer::AddItem(UItemData* ItemData)
{
	for (UItemData* Item : FItemDataRange(ItemData))
	{
		Items.Add(Item);
	}
}

void UItemDataContainer::SetItems(const TArray<UItemData*>& NewItems)
{
	Items = NewItems;
}

bool UItemDataContainer::IsValidData() const
{
	return Items.Num() > 0;
}

void UItemData::SerializeItemData(FArchive& Ar)
{
	Ar << ItemId;
}

void UItemData::InitializeExtraProperties()
{
}

void UItemDataContainer::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);

	int32 ItemNum = 0;

	if (Ar.IsSaving())
	{
		ItemNum = Items.Num();
		Ar << ItemNum;

		for (UItemData* ItemData : Items)
		{
			FItemDataHandle Handle(ItemData);
			Handle.Serialize(Ar);
		}
	}
	else
	{
		Ar << ItemNum;
		
		for (int32 N = 0; N < ItemNum; N++)
		{
			FItemDataHandle Handle;
			Handle.Serialize(Ar);
			Items.Add(Handle.Get());
		}
	}
}

UItemData* UItemDataContainer::GetNextItem()
{
	if (Items.IsValidIndex(ItemIndex))
	{
		return Items[ItemIndex++];
	}
	return nullptr;
}

#if WITH_EDITOR

void UItemDataContainer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UItemDataContainer, Items))
	{
		for (TArray<UItemData*>::TIterator Iterator = Items.CreateIterator(); Iterator; ++Iterator)
		{
			UItemData* Current = *Iterator;
			if (Current && Current->IsA<UItemDataContainer>())
			{
				Iterator.RemoveCurrent();
			}
		}
	}
}

#endif

void UItemDataTradable::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);
	Ar << ItemPrice;
}

void UItemDataEquipment::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);
	
	Ar << Attributes;
}


void UItemDataWeapon::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);
	
	Ar << WeaponClass;

	if (Ar.IsLoading())
	{
		InitializeExtraProperties();
	}
}

UClass* UItemDataWeapon::GetItemClass() const
{
	return WeaponClass;
}

void UItemDataWeapon::AttemptAssignAttributes()
{
	if (!bFixedAttributes)
	{
		AShootWeapon* WeaponCDO = GetItemClass()->GetDefaultObject<AShootWeapon>();
		Attributes = FEquipmentAttributesAssign::AssignAttributes(WeaponCDO->AttributesAssign);
	}
}

void UItemDataWeapon::InitializeExtraProperties()
{
	AShootWeapon* CDO = GetItemClass()->GetDefaultObject<AShootWeapon>();
	RateOfFire = CDO->RateOfFire;
	Magazine = CDO->AmmoNum;
	FireMode = UEnum::GetDisplayValueAsText(CDO->FireMode);
}

void UItemDataModule::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);

	Ar << ModuleClass;

	if (Ar.IsLoading())
	{
		InitializeExtraProperties();
	}
}

UClass* UItemDataModule::GetItemClass() const
{
	return ModuleClass;
}

void UItemDataModule::AttemptAssignAttributes()
{
	if (!bFixedAttributes)
	{
		UDModuleBase* ModuleCDO = GetItemClass()->GetDefaultObject<UDModuleBase>();
		Attributes = FEquipmentAttributesAssign::AssignAttributes(ModuleCDO->AttributesAssign);
	}
}

void UItemDataModule::InitializeExtraProperties()
{
	UDModuleBase* ModuleCDO = GetItemClass()->GetDefaultObject<UDModuleBase>();
	Category = ModuleCDO->Category;
}

UClass* UItemDataExperience::GetItemClass() const
{
	return UDExperience::StaticClass();
}

void UItemDataExperience::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);
	
	Ar << ExperienceAmount;
}

UClass* UItemDataMoney::GetItemClass() const
{
	return UDMoney::StaticClass();
}

void UItemDataMoney::SerializeItemData(FArchive& Ar)
{
	Super::SerializeItemData(Ar);
	
	Ar << MoneyAmount;
}

bool FItemDataHandle::Serialize(FArchive& Ar)
{
	Ar << ItemDataClass;

	if (Ar.IsLoading())
	{
		if (ItemData == nullptr)
		{
			ItemData = NewObject<UItemData>(GetTransientPackage(), ItemDataClass);
		}
	}

	ItemData->SerializeItemData(Ar);
	return true;
}

UItemData* FItemDataHandle::Get() const
{
	return ItemData;
}

bool FQuestConditionHandle::Serialize(FArchive& Ar)
{
	Ar << ConditionClass;
	
	if (Ar.IsLoading())
	{
		if (QuestCondition == nullptr)
		{
			QuestCondition = NewObject<UDQuestCondition>(GetTransientPackage(), ConditionClass);
		}
	}

	QuestCondition->SerializeItemData(Ar);
	return true;
}

UDQuestCondition* FQuestConditionHandle::Get() const
{
	return QuestCondition;
}
