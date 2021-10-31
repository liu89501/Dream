// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "GameFramework/SaveGame.h"
#include "PlayerGameData.generated.h"

#define DEFAULT_SLOT TEXT("GameData")
#define STORE_SLOT TEXT("Store")
#define TASK_SLOT TEXT("Task")

#define MAX_ITEM_NUM 40

USTRUCT()
struct FTaskInformationSaveGame
{
	GENERATED_BODY()

	UPROPERTY()
	int64 TaskId;

	UPROPERTY()
	FSoftObjectPath TaskDescription;

	UPROPERTY()
	FQuestConditionHandle CompleteCondition;

	UPROPERTY()
	FItemDataHandle CompletedReward;

	UPROPERTY()
	ETaskMark TaskMark;
	
	UPROPERTY()
	bool bTracking;

	FTaskInformationSaveGame& operator=(const FTaskInformation& Information)
	{
		TaskId = Information.TaskId;
		TaskDescription = Information.TaskDescription;
		CompletedReward = FItemDataHandle(Information.CompletedReward);
		TaskMark = Information.TaskMark;
		CompleteCondition = FQuestConditionHandle(Information.CompleteCondition);
		bTracking = Information.bTracking;
		return *this;
	}

	FTaskInformationSaveGame(const FTaskInformation& Other)
		: TaskId(Other.TaskId),
		  TaskDescription(Other.TaskDescription),
		  CompleteCondition(Other.CompleteCondition),
		  CompletedReward(Other.CompletedReward),
		  TaskMark(Other.TaskMark),
		  bTracking(Other.bTracking)
	{
	}

	FTaskInformationSaveGame() = default;

	FTaskInformation CastToInformation() const
	{
		FTaskInformation Information;

		Information.TaskId = TaskId;
		Information.TaskDescription = TaskDescription;
		Information.CompletedReward = CompletedReward.Get();
		Information.TaskMark = TaskMark;
		Information.CompleteCondition = CompleteCondition.Get();
		Information.bTracking = bTracking;
		return Information;
	}
};

/**
 * 
 */
UCLASS()
class UPlayerGameData : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FPlayerProperties Properties;

	UPROPERTY()
	TArray<FPlayerWeapon> Weapons;

	UPROPERTY()
	TMap<EModuleCategory, FPlayerModuleList> Modules;

	UPROPERTY()
	TArray<FTalentInfo> TalentList;

	UPROPERTY()
	FSoftObjectPath CharacterMesh;

};

USTRUCT()
struct FItemList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Instanced)
	TArray<UItemData*> Items;
};

USTRUCT()
struct FItemListSaveGame
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FItemDataHandle> Items;
};

UCLASS()
class UStoreData : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TMap<int32, FItemListSaveGame> Stores;
};

UCLASS()
class UTaskData : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FTaskInformationSaveGame> TaskList;
};

UCLASS()
class UStoreList : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TMap<int32, FItemList> Stores;
};

UCLASS()
class UModuleDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FPlayerModule> Modules;
};

UCLASS()
class UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FPlayerWeapon> Weapons;
};

UCLASS()
class UTalentDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FTalentInfo> Talents;
};

UCLASS()
class UTaskDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FTaskInformation> Tasks;
};

