// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlayerDataInterfaceType.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPDS, Log, All);

UENUM(BlueprintType)
enum class EGetWeaponCondition : uint8
{
	All,
	Equipped,
	UnEquipped
};

UENUM(BlueprintType)
namespace EItemType
{
	enum Type
	{
		INVALID,
		Weapon UMETA(DisplayName="武器"),
		Module UMETA(DisplayName="模块"),
		Sundries UMETA(DisplayName="杂物")
	};
}

USTRUCT(BlueprintType)
struct FPlayerWeaponAdd
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponInfo)
	FString WeaponClass;
};

USTRUCT(BlueprintType)
struct FPlayerWeapon
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = WeaponInfo)
	FString WeaponClass;

	UPROPERTY(BlueprintReadOnly, Category = WeaponInfo)
	bool bEquipment;

	UPROPERTY(BlueprintReadOnly, Category = WeaponInfo)
	int32 EquipmentIndex;

	UPROPERTY(BlueprintReadOnly, Category = WeaponInfo)
	int32 WeaponId;
};

USTRUCT(BlueprintType)
struct FDItemInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Item)
	int64 ItemId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Item)
	TEnumAsByte<EItemType::Type> Type;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Item)
	FString ItemClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Item)
	int64 ItemPrice;
};

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 MaxWeaponNum;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int64 Money;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FPlayerWeapon> Weapons;
};

USTRUCT(BlueprintType)
struct FRunServerParameter
{
	GENERATED_USTRUCT_BODY()

	FRunServerParameter() = default;
	FRunServerParameter(const FString& InMapName, const FString& InModeName) :
		MapName(InMapName),
		ModeName(InModeName)
	{
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString MapName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString ModeName;
};