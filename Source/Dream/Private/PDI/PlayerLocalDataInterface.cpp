/*
// ReSharper disable CppExpressionWithoutSideEffects
#include "PDI/PlayerLocalDataInterface.h"
#include "PDI/PlayerGameData.h"
#include "DGameplayStatics.h"
#include "DreamGameInstance.h"
#include "PlayerDataInterfaceStatic.h"

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

template <typename STRUCT>
struct FUniversalPredicate
{
	bool operator()(const STRUCT& A, const STRUCT& B) const
	{
		return A.GetSortNum() < B.GetSortNum();
	}
};

int32 GetMaxExperienceForLevel(int32 Level)
{
	return static_cast<int32>(300 * FMath::Pow(1.1f, Level * 1.1f));
}


FPlayerLocalDataInterface::FPlayerLocalDataInterface(): GameData(nullptr),TaskData(nullptr),StoreData(nullptr)
{
	LoadCounter.Set(3);
	bInitCompleted = true;
}

void FPlayerLocalDataInterface::Initialize()
{
	FPlayerDataInterfaceBase::Initialize();

	if (IsRunningDedicatedServer())
	{
		return;
	}
	
	if (IFileManager::Get().FileExists(*FString::Printf(TEXT("%sSaveGames/%s.sav"), *FPaths::ProjectSavedDir(), DEFAULT_SLOT)))
	{
		FAsyncLoadGameFromSlotDelegate LoadGameDelegate;
		LoadGameDelegate.BindRaw(this, &FPlayerLocalDataInterface::AsyncLoadGameData);
		UGameplayStatics::AsyncLoadGameFromSlot(DEFAULT_SLOT, 0, LoadGameDelegate);
	}
	else
	{
		GameData = NewObject<UPlayerGameData>();

		FPlayerProperties Properties;
		Properties.Money = 666666;
		Properties.MaxExperience = GetMaxExperienceForLevel(1);

		GameData->Properties = Properties;

		GameData->CharacterMesh = FSoftObjectPath(TEXT("/Game/Main/Asset/Skin3"));

		UWeaponDataAsset* WeaponList = LoadObject<UWeaponDataAsset>(nullptr, TEXT("/Game/Main/Asset/Local/DA_Init_Weapons"));
		check(WeaponList);

		for (int N = 0; N < WeaponList->Weapons.Num(); N++)
		{
			FPlayerWeapon Weapon = WeaponList->Weapons[N];

			Weapon.WeaponId = N + 1;
			GameData->Weapons.Add(Weapon);
		}

		UModuleDataAsset* ModuleList = LoadObject<UModuleDataAsset>(nullptr, TEXT("/Game/Main/Asset/Local/DA_Init_Modules"));
		check(ModuleList);

		for (int N = 0; N < ModuleList->Modules.Num(); N++)
		{
			FPlayerModule Module = ModuleList->Modules[N];
			Module.ModuleId = N + 1;
			GameData->Modules.FindOrAdd(Module.Category).Modules.Add(Module);
		}

		UTalentDataAsset* TalentList = LoadObject<UTalentDataAsset>(nullptr, TEXT("/Game/Main/Asset/Local/DA_TalentList"));
		check(ModuleList);

		for (const FTalentInfo& Talent : TalentList->Talents)
		{
			GameData->TalentList.Add(Talent);
		}

		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, 0);
	}

	if (IFileManager::Get().FileExists(*FString::Printf(TEXT("%sSaveGames/%s.sav"), *FPaths::ProjectSavedDir(), TASK_SLOT)))
	{
		FAsyncLoadGameFromSlotDelegate LoadDelegate;
		LoadDelegate.BindRaw(this, &FPlayerLocalDataInterface::AsyncTaskData);
		UGameplayStatics::AsyncLoadGameFromSlot(TASK_SLOT, 0, LoadDelegate);
	}
	else
	{
		TaskData = NewObject<UTaskData>(GetTransientPackage());

		UTaskDataAsset* TaskDataAsset = FPDIStatic::GetTaskDataAsset();
		for (FTaskInformation Information : TaskDataAsset->Tasks)
		{
			Information.CompleteCondition = NewObject<UDQuestCondition>(GetTransientPackage(),
				Information.CompleteCondition->GetClass(), NAME_None, RF_NoFlags, Information.CompleteCondition);

			Information.CompletedReward = NewObject<UItemData>(GetTransientPackage(),
                Information.CompletedReward->GetClass(), NAME_None, RF_NoFlags, Information.CompletedReward);
			
			TaskData->TaskList.Add(Information);
		}
		
		UGameplayStatics::AsyncSaveGameToSlot(TaskData, TASK_SLOT, 0);
	}
	
	if (IFileManager::Get().FileExists(*FString::Printf(TEXT("%sSaveGames/%s.sav"), *FPaths::ProjectSavedDir(), STORE_SLOT)))
	{
		FAsyncLoadGameFromSlotDelegate LoadDelegate;
		LoadDelegate.BindRaw(this, &FPlayerLocalDataInterface::AsyncStoreData);
		UGameplayStatics::AsyncLoadGameFromSlot(STORE_SLOT, 0, LoadDelegate);
	}
	else
	{
		UStoreList* StoreList = LoadObject<UStoreList>(nullptr, TEXT("/Game/Main/Asset/Local/DA_StoreList"));
		StoreData = NewObject<UStoreData>(GetTransientPackage());
			
		int32 ItemId = 1;
		for (const TPair<int32, FItemList>& Data : StoreList->Stores)
		{
			FItemListSaveGame NewList;
			for (UItemData* ItemData : Data.Value.Items)
			{
				UItemData* DupItemData = NewObject<UItemData>(GetTransientPackage(), ItemData->GetClass(), NAME_None, RF_NoFlags, ItemData);
				DupItemData->InitializeExtraProperties();
				DupItemData->ItemId = ItemId++;
				NewList.Items.Add(FItemDataHandle(DupItemData));
			}

			StoreData->Stores.Add(Data.Key, NewList);
		}
		UGameplayStatics::AsyncSaveGameToSlot(StoreData, STORE_SLOT, 0);
	}
}

void FPlayerLocalDataInterface::AsyncLoadGameData(const FString& Slot, const int32 Index, USaveGame* SaveGame)
{
	//UE_LOG(LogTemp, Error, TEXT("GameData: %s"), GameData ? TEXT("Y") : TEXT("N"));
	GameData = Cast<UPlayerGameData>(SaveGame);
	AttemptExecuteInitializeDelegate(GameData ? true : false);
}

void FPlayerLocalDataInterface::AsyncTaskData(const FString& Slot, const int32 Index, USaveGame* SaveGame)
{
	TaskData = Cast<UTaskData>(SaveGame);
	AttemptExecuteInitializeDelegate(TaskData ? true : false);
}

void FPlayerLocalDataInterface::AsyncStoreData(const FString& Slot, const int32 Index, USaveGame* SaveGame)
{
	StoreData = Cast<UStoreData>(SaveGame);
	AttemptExecuteInitializeDelegate(TaskData ? true : false);
}

void FPlayerLocalDataInterface::AttemptExecuteInitializeDelegate(bool bCompleted)
{
	bInitCompleted.AtomicSet(bInitCompleted & bCompleted);
	if (LoadCounter.Decrement() == 0)
	{
		BroadcastOnInitialize(bInitCompleted);
	}
}

void FPlayerLocalDataInterface::AddPlayerRewards(const FItemDataHandle& Rewards)
{
	for (UItemData* ItemData : FItemDataRange(Rewards))
	{
		DoAddItem(ItemData);
	}
	
	BroadcastOnAddPlayerRewards(true);
	UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, 0);
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
		int32 CNum = GameData->Modules.Find(ModuleData->Category)->Modules.Num();

		if (CNum == MAX_ITEM_NUM)
		{
			// todo 添加到仓库
		}
		else
		{
			ModuleData->AttemptAssignAttributes();
			FPlayerModule Module(ModuleData);

			int32 Num = 0;
			for (TPair<EModuleCategory, FPlayerModuleList>& ModulePair : GameData->Modules)
			{
				Num += ModulePair.Value.Modules.Num();
			}
			
			Module.ModuleId = Num + 1;
			GameData->Modules.FindChecked(ModuleData->Category).Modules.Add(Module);
		}
	}
	else if (UItemDataExperience* ExperienceData = Cast<UItemDataExperience>(ItemData))
	{
		DoIncreaseExperience(ExperienceData->ExperienceAmount);
		PlayerDataDelegate.OnPropertiesChange.Broadcast(GameData->Properties);
	}
	else if (UItemDataMoney* ExperienceMoney = Cast<UItemDataMoney>(ItemData))
	{
		FPlatformAtomics::InterlockedAdd(&GameData->Properties.Money, ExperienceMoney->MoneyAmount);
		PlayerDataDelegate.OnPropertiesChange.Broadcast(GameData->Properties);
	}
}

void FPlayerLocalDataInterface::EquipModule(const FEquipModuleParam& Param)
{
	if (GameData)
	{
		FPlayerModule* NewModule = nullptr;
		FPlayerModule* OldModule = nullptr;

		for (TPair<EModuleCategory, FPlayerModuleList>& Module : GameData->Modules)
		{
			for (FPlayerModule& PM : Module.Value.Modules)
			{
				if (Param.ModuleId == PM.ModuleId)
				{
					NewModule = &PM;
				}

				if (Param.ModuleCategory == PM.Category && PM.bEquipped)
				{
					OldModule = &PM;
				}

				if (OldModule && NewModule)
				{
					break;
				}
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

		BroadcastOnEquipModule(true);
		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, 0);
	}
	else
	{
		BroadcastOnEquipModule(false);
	}
}

void FPlayerLocalDataInterface::EquipWeapon(const FEquipWeaponParam& Param)
{
	if (GameData)
	{
		FPlayerWeapon* NewEquippedWeapon = nullptr;
		FPlayerWeapon* OldEquippedWeapon = nullptr;

		for (FPlayerWeapon& Weapon : GameData->Weapons)
		{
			if (Weapon.WeaponId == Param.WeaponId)
			{
				NewEquippedWeapon = &Weapon;
			}

			if (Weapon.Index == Param.EquipmentIndex && Weapon.bEquipped)
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
			NewEquippedWeapon->Index = Param.EquipmentIndex;
		}

		if (OldEquippedWeapon)
		{
			OldEquippedWeapon->bEquipped = false;
		}

		BroadcastOnEquipWeapon(true);
		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, 0);
	}
	else
	{
		BroadcastOnEquipWeapon(false);
	}
}

void FPlayerLocalDataInterface::GetStoreItems(int32 StoreId)
{
	FStoreInformation StoreInformation;
	if (FItemListSaveGame* ItemList = StoreData->Stores.Find(StoreId))
	{
		for (const FItemDataHandle& Handle : ItemList->Items)
		{
			StoreInformation.Items.Add(Handle);
		}
		
		BroadcastOnGetStoreItems(StoreInformation, true);
	}
	else
	{
		BroadcastOnGetStoreItems(StoreInformation, false);
	}
}

void FPlayerLocalDataInterface::PayItem(const FBuyItemParam& Param)
{
	UItemDataTradable* BuyItem = nullptr;

	if (FItemListSaveGame* ItemList = StoreData->Stores.Find(Param.StoreId))
	{
		for (const FItemDataHandle& Information : ItemList->Items)
		{
			if (Information.Get()->ItemId == Param.ItemId)
			{
				BuyItem = Cast<UItemDataTradable>(Information.Get());
				break;
			}
		}
	}

	if (BuyItem)
	{
		if (GameData->Properties.Money >= BuyItem->ItemPrice)
		{
			DoAddItem(BuyItem);

			FPlatformAtomics::InterlockedAdd(&GameData->Properties.Money, -BuyItem->ItemPrice);
			UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, 0);

			PlayerDataDelegate.OnPropertiesChange.Broadcast(GameData->Properties);
					
			BroadcastOnBuyItem(true);
			return;
		}
	}

	BroadcastOnBuyItem(false);
}

void FPlayerLocalDataInterface::GetPlayerInfo(EGetEquipmentCondition Condition)
{
	FPlayerInfo PlayerInfo;
	if (GameData)
	{
		FilterPlayerData(FPlayerWeapon, bEquipped, GameData->Weapons, Condition, PlayerInfo.Weapons);
		
		FilterPlayerData(FTalentInfo, bLearned, GameData->TalentList, Condition, PlayerInfo.Talents);

		for (TPair<EModuleCategory, FPlayerModuleList>& Module : GameData->Modules)
		{
			for (const FPlayerModule& PM : Module.Value.Modules)
			{
				if (Condition == EGetEquipmentCondition::All)
				{
					PlayerInfo.Modules.Add(PM);
					continue;
				}
					
				if (PM.bEquipped)
				{
					if (Condition == EGetEquipmentCondition::Equipped)
					{
						PlayerInfo.Modules.Add(PM);
					}	
				}
				else
				{
					if (Condition == EGetEquipmentCondition::UnEquipped)
					{
						PlayerInfo.Modules.Add(PM);
					}
				}
			}
		}

		PlayerInfo.Properties = GameData->Properties;
		PlayerInfo.CharacterMesh = GameData->CharacterMesh;

		BroadcastOnGetPlayerInfo(PlayerInfo, true);
	}
	else
	{
		BroadcastOnGetPlayerInfo(PlayerInfo, false);
	}
}

void FPlayerLocalDataInterface::GetTalents(ETalentCategory TalentCategory)
{
	TArray<FTalentInfo> TalentInfos;
	if (GameData)
	{
		TalentInfos = GameData->TalentList;
		TalentInfos.Sort<FTalentPredicate>(FTalentPredicate());

		BroadcastOnGetTalents(TalentInfos, true);
	}
	else
	{
		BroadcastOnGetTalents(TalentInfos, false);
	}
}

void FPlayerLocalDataInterface::LearningTalents(const TArray<int32>& TalentIdArray)
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

		BroadcastOnLearnTalents(true);

		UGameplayStatics::AsyncSaveGameToSlot(GameData, DEFAULT_SLOT, 0);
	}
	else
	{
		BroadcastOnLearnTalents(false);
	}
}

void FPlayerLocalDataInterface::GetTasks(const FSearchTaskParam& Param)
{
	TArray<FTaskInformation> FilterPostTask;
	if (TaskData)
	{
		FilterTasks(Param.Condition, FilterPostTask);

		BroadcastOnGetTasks(FilterPostTask, true);
	}
	else
	{
		BroadcastOnGetTasks(FilterPostTask, true);
	}
}

void FPlayerLocalDataInterface::FilterTasks(EGetTaskCondition Condition, TArray<FTaskInformation>& Tasks) const
{
	for (const FTaskInformationSaveGame& TaskInformation : TaskData->TaskList)
	{
		if (Condition == EGetTaskCondition::Submitted)
		{
			if (TaskInformation.TaskMark == ETaskMark::Submitted)
			{
				Tasks.Add(TaskInformation.CastToInformation());
			}
		}
		else if (Condition == EGetTaskCondition::UnSubmit)
		{
			if (TaskInformation.TaskMark == ETaskMark::Completed || TaskInformation.TaskMark == ETaskMark::Accepted)
			{
				Tasks.Add(TaskInformation.CastToInformation());
			}
		}
		else if (Condition == EGetTaskCondition::NotAccept)
		{
			if (TaskInformation.TaskMark == ETaskMark::NotAccept)
			{
				Tasks.Add(TaskInformation.CastToInformation());
			}
		}
		else
		{
			
		}
	}
}

void FPlayerLocalDataInterface::DeliverTask(int32 TaskId)
{
	int32 Tid = TaskId - 1;
	if (TaskData->TaskList.IsValidIndex(Tid))
	{
		FTaskInformationSaveGame& Information = TaskData->TaskList[Tid];

		bool bIsCompleted = Information.CompleteCondition->IsCompleted();
		bool bIsNotSubmit = Information.TaskMark != ETaskMark::Submitted;

		if (bIsCompleted && bIsNotSubmit)
		{
			Information.TaskMark = ETaskMark::Submitted;
				
			for (UItemData* ItemData : FItemDataRange(Information.CompletedReward))
			{
				DoAddItem(ItemData);
			}

			BroadcastOnDeliverTask(Information.CompletedReward.Get(), true);
				
			UGameplayStatics::AsyncSaveGameToSlot(TaskData, TASK_SLOT, 0);

			return;
		}
	}

	BroadcastOnDeliverTask(nullptr, false);
}

void FPlayerLocalDataInterface::AcceptTask(const FAcceptTaskParam& Param)
{
	int32 Tid = Param.TaskId - 1;
	if (TaskData->TaskList.IsValidIndex(Tid))
	{
		TaskData->TaskList[Tid].TaskMark = ETaskMark::Accepted;
		UGameplayStatics::AsyncSaveGameToSlot(TaskData, TASK_SLOT, 0);

		BroadcastOnAcceptTask(true);
	}
	else
	{
		BroadcastOnAcceptTask(false);
	}
}

void FPlayerLocalDataInterface::ModifyTrackingState(const FModifyTrackingParam& Param)
{
	int32 Tid = Param.TaskId - 1;
	if (TaskData->TaskList.IsValidIndex(Tid))
	{
		TaskData->TaskList[Tid].bTracking = Param.bTracking;

		UGameplayStatics::AsyncSaveGameToSlot(TaskData, TASK_SLOT, 0);

		BroadcastOnModifyTaskTracking(true);
	}
	else
	{
		BroadcastOnModifyTaskTracking(false);
	}
}

void FPlayerLocalDataInterface::UpdateTaskState(const FQuestActionHandle& Handle)
{
	for (FTaskInformationSaveGame& Information : TaskData->TaskList)
	{
		if (Information.TaskMark == ETaskMark::Accepted)
		{
			if (Information.CompleteCondition->UpdateCondition(Handle))
			{
				Information.TaskMark = ETaskMark::Completed;
			}
		}
	}

	UGameplayStatics::AsyncSaveGameToSlot(TaskData, TASK_SLOT, 0);

	TArray<FTaskInformation> FilterPostTask;
	FilterTasks(EGetTaskCondition::UnSubmit, FilterPostTask);

	PlayerDataDelegate.OnTaskStateChanged.Broadcast(FilterPostTask);
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

const FPlayerProperties& FPlayerLocalDataInterface::GetCachedProperties() const
{
	return GameData->Properties;
}

void FPlayerLocalDataInterface::AddReferencedObjects(FReferenceCollector& Collector)
{
	// 如果不添加这行代码则无法预测GameData会在什么时候被GC掉而导致一系列BUG
	Collector.AddReferencedObject(GameData);
	Collector.AddReferencedObject(TaskData);
	Collector.AddReferencedObject(StoreData);
}
*/
