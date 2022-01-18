// Fill out your copyright notice in the Description page of Project Settings.


#include "PDI/PDIFunctions.h"
#include "EngineUtils.h"
#include "ShootWeapon.h"
#include "DModuleBase.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"
#include "GameFramework/PlayerState.h"

const FPlayerProperties& UPDIFunctions::GetCachedPlayerProperties()
{
	return FPDIStatic::Get()->GetCachedProperties();
}

int32 UPDIFunctions::GetPlayerMaterialNum(int32 ItemGuid)
{
	const FMaterialsHandle& MaterialsHandle = FPDIStatic::Get()->GetMaterialsHandle();
	if (MaterialsHandle.IsValid())
	{
		if (int32* Num = MaterialsHandle->Find(ItemGuid))
		{
			return *Num;
		}
	}

	return 0;
}

void UPDIFunctions::SendUpdateTaskCondForEvent(APlayerController* PlayerCtrl, FName EventName)
{
	if (PlayerCtrl && PlayerCtrl->PlayerState)
	{
		TSharedRef<FQuestAction_Event> ActionEvent = MakeShared<FQuestAction_Event>(EventName);
		FPDIStatic::Get()->UpdateTaskState(FQuestActionHandle(PlayerCtrl->PlayerState->GetPlayerId(), ActionEvent));
	}
}

void UPDIFunctions::SendUpdateTaskCondForEventToAll(UObject* WorldContextObject, FName EventName)
{
	if (IsRunningDedicatedServer())
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			for (TActorIterator<APlayerController> It(World); It; ++It)
			{
				SendUpdateTaskCondForEvent(*It, EventName);
			}
		}
	}
}

void UPDIFunctions::BindPropertiesChangeDelegate(FPDIFOnPropertiesChange Delegate, FMulticastDelegateHandle& Handle)
{
	Handle.Handle = FPDIStatic::Get()->GetPlayerDataDelegate().OnPropertiesChange.AddUFunction(Delegate.GetUObject(), Delegate.GetFunctionName());
}

void UPDIFunctions::RemovePropertiesDelegateHandle(const FMulticastDelegateHandle& MulticastDelegateHandle)
{
	FPDIStatic::Get()->GetPlayerDataDelegate().OnPropertiesChange.Remove(MulticastDelegateHandle.Handle);
}

void UPDIFunctions::GroupModules(const TArray<FPlayerModule>& Modules, TMap<EModuleCategory, FPlayerModuleList>& GroupModules)
{
	for (const FPlayerModule& PM : Modules)
	{
		GroupModules.FindOrAdd(PM.Category).Modules.Add(PM);
	}
}

void UPDIFunctions::GetEquippedModule(const TArray<FPlayerModule>& Modules, TArray<FPlayerModule>& EquippedModules)
{
	for (const FPlayerModule& PM : Modules)
	{
		if (PM.bEquipped)
		{
			EquippedModules.Add(PM);
		}
	}
}

bool UPDIFunctions::IsConditionCompleted(const FTaskInformation& Task)
{
	bool bCompleted = true;
	for (UDQuestCondition* QuestCondition : Task.Condition.Conditions)
	{
		bCompleted &= QuestCondition->IsCompleted();
	}

	return bCompleted;
}

float UPDIFunctions::GetConditionPercentage(const FQuestConditionHandle& Handle)
{
	if (Handle.Conditions.Num() == 0)
	{
		return 0;
	}
	
	float PercentAVG = 0.f;

	for (UDQuestCondition* Condition : Handle.Conditions)
	{
		PercentAVG += Condition->GetQuestProgressPercent();
	}

	return PercentAVG / Handle.Conditions.Num();
}

TSubclassOf<AShootWeapon> UPDIFunctions::SoftClassPathToWeaponClass(const FSoftClassPath& SoftClassPath)
{
	return SoftClassPath.TryLoadClass<AShootWeapon>();
}

TSubclassOf<UDModuleBase> UPDIFunctions::SoftClassPathToModuleClass(const FSoftClassPath& SoftClassPath)
{
	return SoftClassPath.TryLoadClass<UDModuleBase>();
}

UDQuestDescription* UPDIFunctions::SoftObjectToDQuestDescription(const FSoftObjectPath& ObjectPath)
{
	return Cast<UDQuestDescription>(ObjectPath.TryLoad());
}

void UPDIFunctions::MakeItemArray(const FItemListHandle& ItemListHandle, TArray<FItemHandle>& ItemArray)
{
	for (const TSharedPtr<FItem>& Item : ItemListHandle)
	{
		ItemArray.Add(Item);
	}
}

FItemHandle UPDIFunctions::MakeSimpleItemHandle(FItemGuidHandle Guid, int32 Num)
{
	return FItemHandle(MakeShared<FItemSimple>(Guid.ItemGuid, Num));
}
