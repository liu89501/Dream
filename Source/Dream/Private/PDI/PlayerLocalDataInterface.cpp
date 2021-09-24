// ReSharper disable CppExpressionWithoutSideEffects
#include "PDI/PlayerLocalDataInterface.h"
#include "DQuestCondition.h"
#include "PDI/PlayerGameData.h"
#include "Kismet/GameplayStatics.h"

#define FilterPlayerData(DataType, CPropName, DataArray, Condition, Out) for (DataType _PData : DataArray) \
    { \
    	if (Condition == EGetEquipmentCondition::Equipped) \
    	{ \
    		if (_PData.CPropName) \
    		{ \
    			Out.Add(_PData); \
    		} \
    	} \
    	else if (Condition == EGetEquipmentCondition::UnEquipped) \
    	{ \
    		if (!_PData.CPropName) \
    		{ \
    			Out.Add(_PData); \
    		} \
    	} \
    	else \
    	{ \
    		Out.Add(_PData); \
    	} \
    }


struct FTalentPredicate
{
	bool operator()(const FTalentInfo& A, const FTalentInfo& B) const
	{
		return A.TalentIndex < B.TalentIndex;
	}
};

struct FTaskPredicate
{
	bool operator()(const FTaskInformation& A, const FTaskInformation& B) const
	{
		return A.TaskId < B.TaskId;
	}
};

int32 GetMaxExperienceForLevel(int32 Level)
{
	return static_cast<int32>(300 * FMath::Pow(1.1f, Level * 1.1f));
}


FPlayerLocalDataInterface::FPlayerLocalDataInterface(): GameData(nullptr),TaskData(nullptr)
{
}

void FPlayerLocalDataInterface::Initialize(FInitializeDelegate Delegate)
{
	FAsyncLoadGameFromSlotDelegate LoadGameDelegate;
	LoadGameDelegate.BindRaw(this, &FPlayerLocalDataInterface::AsyncLoadGameData, Delegate);
	UGameplayStatics::AsyncLoadGameFromSlot(DEFAULT_SLOT, DEFAULT_SLOT_INDEX, LoadGameDelegate);
	
	if (!IFileManager::Get().FileExists(*FString::Printf(TEXT("%sSaveGames/%s.sav"), *FPaths::ProjectSavedDir(), STORE_SLOT)))
	{
		UStoreList* StoreList = LoadObject<UStoreList>(GetTransientPackage(), TEXT("/Game/Main/Asset/Test/DA_StoreList"));

		for (const FStoreDataInfo& Data : StoreList->Stores)
		{
			UStoreData* StoreData = NewObject<UStoreData>(GetTransientPackage());
			for (UItemData* ItemData : Data.Items)
			{
				ItemData->InitializeProperties();
			}
			StoreData->Items.Append(Data.Items);
			UGameplayStatics::AsyncSaveGameToSlot(StoreData, STORE_SLOT, Data.StoreId);
		}
	}
}

void FPlayerLocalDataInterface::AsyncLoadGameData(const FString& Slot, const int32 Index, USaveGame* SaveGame, FInitializeDelegate Delegate)
{
	GameData = Cast<UPlayerGameData>(SaveGame);

	//UE_LOG(LogTemp, Error, TEXT("GameData: %s"), GameData ? TEXT("Y") : TEXT("N"));

	if (GameData == nullptr)
	{
		GameData = Cast<UPlayerGameData>(UGameplayStatics::CreateSaveGameObject(UPlayerGameData::StaticClass()));

		FPlayerProperties Properties;
		Properties.Money = 666666;
		Properties.AvailableTalentPoints = 0;
		Properties.MaxExperience = GetMaxExperienceForLevel(1);

		GameData->Properties = Properties;

		UWeaponDataAsset* WeaponList = LoadObject<UWeaponDataAsset>(GetTransientPackage(), TEXT("/Game/Main/Asset/Test/DA_WeaponList"));
		check(WeaponList);

		for (const FPlayerWeapon& Weapon : WeaponList->Weapons)
		{
			GameData->Weapons.Add(Weapon);
		}

		UModuleDataAsset* ModuleList = LoadObject<UModuleDataAsset>(GetTransientPackage(), TEXT("/Game/Main/Asset/Test/DA_ModuleList"));
		check(ModuleList);

		for (const FPlayerModule& Module : ModuleList->Modules)
		{
			GameData->Modules.Add(Module);
		}

		UTalentDataAsset* TalentList = LoadObject<UTalentDataAsset>(GetTransientPackage(), TEXT("/Game/Main/Asset/Test/DA_TalentList"));
		check(ModuleList);

		for (const FTalentInfo& Talent : TalentList->Talents)
		{
			GameData->TalentList.Add(Talent);
		}

		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, DEFAULT_SLOT_INDEX);
	}

	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindRaw(this, &FPlayerLocalDataInterface::AsyncTaskData, Delegate);
	UGameplayStatics::AsyncLoadGameFromSlot(TASK_SLOT, TASK_SLOT_INDEX, LoadDelegate);
}

void FPlayerLocalDataInterface::AsyncTaskData(const FString& Slot, const int32 Index, USaveGame* SaveGame, FInitializeDelegate Delegate)
{
	TaskData = Cast<UTaskData>(SaveGame);

	if (TaskData == nullptr)
	{
		UTaskDataAsset* TaskList = LoadObject<UTaskDataAsset>(GetTransientPackage(), TEXT("/Game/Main/Asset/Test/DA_TaskList"));
		TaskData = NewObject<UTaskData>(GetTransientPackage());
		TaskData->TaskList.Append(TaskList->Tasks);
		TaskData->TaskList.Sort(FTaskPredicate());
		UGameplayStatics::AsyncSaveGameToSlot(TaskData, TASK_SLOT, TASK_SLOT_INDEX);
	}

	Delegate.ExecuteIfBound(true);
}

TMap<EModuleCategory, FPlayerModuleList> FPlayerLocalDataInterface::GetModuleFromCategory() const
{
	TMap<EModuleCategory, FPlayerModuleList> Modules;

	for (FPlayerModule& Module : GameData->Modules)
	{
		Modules.FindOrAdd(Module.Category).Modules.Add(Module);
	}

	return Modules;
}


void FPlayerLocalDataInterface::AddPlayerRewards(const TArray<UItemData*>& Rewards, FCommonCompleteNotify Delegate)
{
	for (UItemData* ItemData : Rewards)
	{
		DoAddItem(ItemData);
	}
	
	Delegate.ExecuteIfBound(MSG_SUCCESS);
	UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, DEFAULT_SLOT_INDEX);
}

void FPlayerLocalDataInterface::DoAddItem(UItemData* ItemData) const
{
	if (UItemDataWeapon* WeaponData = Cast<UItemDataWeapon>(ItemData))
	{
		int32 Num = GameData->Weapons.Num();

		if (Num == MAX_ITEM_NUM)
		{
			// todo 添加到仓库
		}
		else
		{
			WeaponData->AttemptAssignAttributes();
			FPlayerWeapon Weapon(WeaponData);
			Weapon.WeaponId = Num + 1;
			GameData->Weapons.Add(Weapon);
		}
	}
	else if (UItemDataModule* ModuleData = Cast<UItemDataModule>(ItemData))
	{
		TMap<EModuleCategory, FPlayerModuleList> Modules = GetModuleFromCategory();

		int32 CNum = Modules.Find(ModuleData->Category)->Modules.Num();

		if (CNum == MAX_ITEM_NUM)
		{
			// todo 添加到仓库
		}
		else
		{
			ModuleData->AttemptAssignAttributes();
			FPlayerModule Module(ModuleData);
			Module.ModuleId = GameData->Modules.Num() + 1;
			GameData->Modules.Add(Module);
		}
	}
	else if (UItemDataExperience* ExperienceData = Cast<UItemDataExperience>(ItemData))
	{
		DoIncreaseExperience(ExperienceData->ExperienceAmount);
	}
	else if (UItemDataMoney* ExperienceMoney = Cast<UItemDataMoney>(ItemData))
	{
		FPlatformAtomics::InterlockedAdd(&GameData->Properties.Money, ExperienceMoney->MoneyAmount);
	}
}

void FPlayerLocalDataInterface::EquipModule(int64 ModuleId, EModuleCategory ModuleCategory, FCommonCompleteNotify Delegate)
{
	if (GameData)
	{
		FPlayerModule* NewModule = nullptr;
		FPlayerModule* OldModule = nullptr;

		for (FPlayerModule& Module : GameData->Modules)
		{
			if (ModuleId == Module.ModuleId)
			{
				NewModule = &Module;
			}

			if (ModuleCategory == Module.Category && Module.bEquipped)
			{
				OldModule = &Module;
			}

			if (OldModule && NewModule)
			{
				break;
			}
		}

		if (NewModule)
		{
			NewModule->bEquipped = true;
		}

		if (OldModule)
		{
			OldModule->bEquipped = false;
		}

		Delegate.ExecuteIfBound(MSG_SUCCESS);
		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, DEFAULT_SLOT_INDEX);
	}
	else
	{
		Delegate.ExecuteIfBound(MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::LearningTalent(int32 TalentId, FCommonCompleteNotify Delegate)
{
}

void FPlayerLocalDataInterface::EquipWeapon(int64 WeaponId, int32 EquippedIndex, FCommonCompleteNotify Delegate)
{
	if (GameData)
	{
		FPlayerWeapon* NewEquippedWeapon = nullptr;
		FPlayerWeapon* OldEquippedWeapon = nullptr;

		for (FPlayerWeapon& Weapon : GameData->Weapons)
		{
			if (Weapon.WeaponId == WeaponId)
			{
				NewEquippedWeapon = &Weapon;
			}

			if (Weapon.Index == EquippedIndex && Weapon.bEquipped)
			{
				OldEquippedWeapon = &Weapon;
			}

			if (NewEquippedWeapon && OldEquippedWeapon)
			{
				break;
			}
		}

		if (NewEquippedWeapon)
		{
			NewEquippedWeapon->bEquipped = true;
			NewEquippedWeapon->Index = EquippedIndex;
		}

		if (OldEquippedWeapon)
		{
			OldEquippedWeapon->bEquipped = false;
		}

		Delegate.ExecuteIfBound(MSG_SUCCESS);
		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, DEFAULT_SLOT_INDEX);
	}
	else
	{
		Delegate.ExecuteIfBound(MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::GetStoreItems(int32 StoreId, FGetStoreItemsComplete Delegate)
{
	FAsyncLoadGameFromSlotDelegate AsyncDelegate;

	AsyncDelegate.BindLambda([Delegate, this](const FString& Slot, const int32 Index, USaveGame* SaveGame)
	{
		if (UStoreData* StoreData = Cast<UStoreData>(SaveGame))
		{
			FStoreInformation StoreInformation;
			StoreInformation.Items = StoreData->Items;
			StoreInformation.PlayerMoneyAmount = GameData->Properties.Money;
			Delegate.ExecuteIfBound(StoreInformation, MSG_SUCCESS);
		}
		else
		{
			FStoreInformation Dummy;
			Delegate.ExecuteIfBound(Dummy, MSG_ERROR);
		}
	});

	UGameplayStatics::AsyncLoadGameFromSlot(STORE_SLOT, StoreId - 1, AsyncDelegate);
}

void FPlayerLocalDataInterface::RunServer(const FRunServerParameter& Parameter, FGetServerComplete Delegate)
{
}

void FPlayerLocalDataInterface::PayItem(int32 StoreId, int64 ItemId, FCommonCompleteNotify Delegate)
{
	FAsyncLoadGameFromSlotDelegate AsyncDelegate;

	AsyncDelegate.BindLambda([ItemId, Delegate, this](const FString& Slot, const int32 Index, USaveGame* SaveGame)
	{
		if (UStoreData* StoreData = Cast<UStoreData>(SaveGame))
		{
			UItemData* BuyItem = nullptr;

			for (UItemData* Information : StoreData->Items)
			{
				if (Information->ItemId == ItemId)
				{
					BuyItem = Information;
					break;
				}
			}

			if (BuyItem)
			{
				if (GameData->Properties.Money >= BuyItem->ItemPrice)
				{
					DoAddItem(BuyItem);

					FPlatformAtomics::InterlockedExchange(&GameData->Properties.Money, GameData->Properties.Money - BuyItem->ItemPrice);
					UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, DEFAULT_SLOT_INDEX);
					Delegate.ExecuteIfBound(MSG_SUCCESS);
					return;
				}
			}
		}

		Delegate.ExecuteIfBound(MSG_ERROR);
	});

	UGameplayStatics::AsyncLoadGameFromSlot(STORE_SLOT, StoreId - 1, AsyncDelegate);
}

void FPlayerLocalDataInterface::GetPlayerWeapons(EGetEquipmentCondition Condition, FGetWeaponComplete Delegate)
{
	TArray<FPlayerWeapon> Weapons;

	if (GameData)
	{
		FilterPlayerData(FPlayerWeapon, bEquipped, GameData->Weapons, Condition, Weapons);
		Delegate.ExecuteIfBound(Weapons, MSG_SUCCESS);;
	}
	else
	{
		Delegate.ExecuteIfBound(Weapons, MSG_ERROR);;
	}
}

void FPlayerLocalDataInterface::GetPlayerInfo(EGetEquipmentCondition Condition, FGetPlayerInfoComplete Delegate)
{
	FPlayerInfo PlayerInfo;
	if (GameData)
	{
		FilterPlayerData(FPlayerWeapon, bEquipped, GameData->Weapons, Condition, PlayerInfo.Weapons);
		FilterPlayerData(FPlayerModule, bEquipped, GameData->Modules, Condition, PlayerInfo.Modules);
		FilterPlayerData(FTalentInfo, bLearned, GameData->TalentList, Condition, PlayerInfo.Talents);

		PlayerInfo.Properties = GameData->Properties;

		Delegate.ExecuteIfBound(PlayerInfo, MSG_SUCCESS);
	}
	else
	{
		Delegate.ExecuteIfBound(PlayerInfo, MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::GetTalents(EPDTalentCategory::Type TalentCategory, FGetTalentsComplete Delegate)
{
	TArray<FTalentInfo> TalentInfos;
	if (GameData)
	{
		TalentInfos = GameData->TalentList;
		TalentInfos.Sort<FTalentPredicate>(FTalentPredicate());

		Delegate.ExecuteIfBound(TalentInfos, MSG_SUCCESS);
	}
	else
	{
		Delegate.ExecuteIfBound(TalentInfos, MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::GetPlayerProperties(FGetPlayerPropertiesDelegate Delegate)
{
	if (GameData)
	{
		Delegate.ExecuteIfBound(GameData->Properties, MSG_SUCCESS);
	}
	else
	{
		FPlayerProperties Dummy;
		Delegate.ExecuteIfBound(Dummy, MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::LearningTalents(const TArray<int32>& TalentIdArray, FCommonCompleteNotify Delegate)
{
	if (GameData)
	{
		TMap<int32, bool> TalentLearnedMap;
		for (int32 TalentId : TalentIdArray)
		{
			TalentLearnedMap.Add(TalentId, true);
		}

		for (FTalentInfo& Talent : GameData->TalentList)
		{
			Talent.bLearned = TalentLearnedMap.Find(Talent.TalentId) ? true : false;
		}

		Delegate.ExecuteIfBound(MSG_SUCCESS);

		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, DEFAULT_SLOT_INDEX);
	}
	else
	{
		Delegate.ExecuteIfBound(MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::GetTasks(EGetTaskCondition Condition, FGetTasksDelegate Delegate)
{
	if (TaskData)
	{
		if (Condition == EGetTaskCondition::All)
		{
			Delegate.ExecuteIfBound(TaskData->TaskList, MSG_SUCCESS);
		}
		else
		{
			TArray<FTaskInformation> FilterPostTask;
			for (const FTaskInformation& TaskInformation : TaskData->TaskList)
			{
				if (Condition == EGetTaskCondition::Completed)
				{
					if (TaskInformation.bIsCompleted)
					{
						FilterPostTask.Add(TaskInformation);
					}
				}
				else if (Condition == EGetTaskCondition::InProgress)
				{
					if (TaskInformation.bIsInProgress)
					{
						FilterPostTask.Add(TaskInformation);
					}
				}
				else if (Condition == EGetTaskCondition::UnComplete)
				{
					if (!TaskInformation.bIsCompleted)
					{
						FilterPostTask.Add(TaskInformation);
					}
				}
			}

			Delegate.ExecuteIfBound(FilterPostTask, MSG_SUCCESS);
		}
	}
	else
	{
		TArray<FTaskInformation> Dummy;
		Delegate.ExecuteIfBound(Dummy, MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::DeliverTask(const int64& TaskId, FCommonCompleteNotify Delegate)
{
	FString ErrorMessage;
	
	if (TaskData)
	{
		int64 Tid = FMath::Max(TaskId - 1, TaskId);
		if (TaskData->TaskList.IsValidIndex(Tid))
		{
			FTaskInformation& Information = TaskData->TaskList[Tid];
			if (Information.CompleteCondition->IsCompleted())
			{
				for (UItemData* ItemData : Information.CompletedReward)
				{
					DoAddItem(ItemData);
				}
			}
			else
			{
				ErrorMessage = TEXT("Task UnComplete");
			}
		}
		else
		{
			ErrorMessage = TEXT("TaskId Invalid");
		}
	}
	else
	{
		ErrorMessage = MSG_ERROR;
	}

	Delegate.ExecuteIfBound(ErrorMessage);
}

void FPlayerLocalDataInterface::IncreaseExperience(const FUserExperiencePair& UserExperience, FExperienceChangeDelegate Delegate)
{
	if (GameData)
	{
		DoIncreaseExperience(UserExperience.IncreaseExperienceAmount);
		Delegate.ExecuteIfBound(GameData->Properties.Level, MSG_SUCCESS);
	}
	else
	{
		Delegate.ExecuteIfBound(0, MSG_ERROR);
	}
}

void FPlayerLocalDataInterface::DoIncreaseExperience(int32 ExpAmount) const
{
	int32 RemainExp = GameData->Properties.MaxExperience - GameData->Properties.CurrentExperience;

	if (RemainExp <= ExpAmount)
	{
		int32 NewExp = ExpAmount - RemainExp;
		int32 NextMaxExperience = GetMaxExperienceForLevel(GameData->Properties.Level);

		FPlatformAtomics::InterlockedIncrement(&GameData->Properties.Level);
		GameData->Properties.MaxExperience = NextMaxExperience;
		GameData->Properties.CurrentExperience = FMath::Min(NewExp, NextMaxExperience);

		if (NewExp >= NextMaxExperience)
		{
			DoIncreaseExperience(NewExp - NextMaxExperience);
		}
	}
	else
	{
		FPlatformAtomics::InterlockedAdd(&GameData->Properties.CurrentExperience, ExpAmount);
	}
}


void FPlayerLocalDataInterface::RegisterServer(int32 Port, int32 MaxPlayers, const FString& MapName, FRegisterServerComplete Delegate)
{
}

void FPlayerLocalDataInterface::UnRegisterServer()
{
}

void FPlayerLocalDataInterface::UpdateActivePlayers(bool bIncrement)
{
}

void FPlayerLocalDataInterface::Login(FCommonCompleteNotify Delegate)
{
	Delegate.ExecuteIfBound(MSG_SUCCESS);
}

void FPlayerLocalDataInterface::Logout()
{
}

FString FPlayerLocalDataInterface::GetServerToken() const
{
	return TEXT("NONE");
}

const FPlayerProperties& FPlayerLocalDataInterface::GetCachedProperties() const
{
	return GameData->Properties;
}

void FPlayerLocalDataInterface::AddReferencedObjects(FReferenceCollector& Collector)
{
	// 如果不添加这行代码则无法预测GameData会在什么时候被GC掉而导致一系列BUG
	Collector.AddReferencedObject(GameData);
	Collector.AddReferencedObject(TaskData);
}
