// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "GameFramework/SaveGame.h"
#include "PlayerGameData.generated.h"

#define DEFAULT_SLOT TEXT("GameData")
#define DEFAULT_SLOT_INDEX 0

#define STORE_SLOT TEXT("Store")

#define TASK_SLOT TEXT("Task")
#define TASK_SLOT_INDEX 0

#define MAX_ITEM_NUM 40

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
	TArray<FPlayerModule> Modules;

	UPROPERTY()
	TArray<FTalentInfo> TalentList;

};

UCLASS()
class UStoreData : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<UItemData*> Items;
};

UCLASS()
class UTaskData : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FTaskInformation> TaskList;
};

USTRUCT()
struct FStoreDataInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 StoreId;

	UPROPERTY(EditAnywhere, Instanced)
	TArray<UItemData*> Items;
};

UCLASS()
class UStoreList : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FStoreDataInfo> Stores;
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

