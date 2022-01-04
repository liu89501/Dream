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

int32 UPDIFunctions::GetCacheItemCount(int32 ItemGuid)
{
	return FPDIStatic::Get()->GetCacheItemCount(ItemGuid);
}

void UPDIFunctions::DecreaseCacheItemCount(int32 ItemGuid, int32 DecValue)
{
	FPDIStatic::Get()->IncreaseItemCount(ItemGuid, -DecValue);
}

void UPDIFunctions::IncreaseCacheItemCount(int32 ItemGuid, int32 IncValue)
{
	FPDIStatic::Get()->IncreaseItemCount(ItemGuid, IncValue);
}

void UPDIFunctions::UpdateCacheItemCount(const TArray<FAcquisitionCost>& Costs)
{
	FPlayerDataInterface* Interface = FPDIStatic::Get();
	for (const FAcquisitionCost& Cost : Costs)
	{
		Interface->IncreaseItemCount(Cost.ItemGuid, -Cost.CostAmount);
	}
}

void UPDIFunctions::SendUpdateTaskCondForEvent(APlayerController* PlayerCtrl, FName EventName)
{
	if (PlayerCtrl && PlayerCtrl->PlayerState)
	{
		FQuestAction_Event ActionEvent(EventName);
		FPDIStatic::Get()->UpdateTaskState(FQuestActionHandle(PlayerCtrl->PlayerState->GetPlayerId(), &ActionEvent));
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

void UPDIFunctions::TiledCondition(const FQuestConditionHandle& Condition, TArray<UDQuestCondition*>& Conditions)
{
	if (UDQuestCondition_Container* Container = Cast<UDQuestCondition_Container>(*Condition))
	{
		Conditions = Container->GetConditions();
	}
	else
	{
		Conditions.Add(*Condition);
	}
}

bool UPDIFunctions::IsConditionCompleted(const FTaskInformation& Task)
{
	TArray<UDQuestCondition*> Conditions;
	TiledCondition(Task.Condition, Conditions);

	bool bCompleted = true;
	for (UDQuestCondition* QuestCondition : Conditions)
	{
		bCompleted &= QuestCondition->IsCompleted();
	}

	return bCompleted;
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
