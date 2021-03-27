// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RSA.h"
#include "PlayerDataStoreType.generated.h"

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
		Weapon,
		Gear,
		Sundries
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
struct FStoreItem
{
	GENERATED_USTRUCT_BODY()

public:

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

public:

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

public:

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

struct FAESUtils
{
public:

	FAESUtils() = default;

	FAESUtils(FString InSecretKey)
	{
		FBase64::Decode(InSecretKey, SecretKey);
	}

	void EncryptData(const FString& Content, FString& Encrypt)
	{
		FTCHARToUTF8 UTF8Content(*Content);
		const uint8* ContentPtr = reinterpret_cast<const uint8*>(UTF8Content.Get());
		TArray<uint8> Data(ContentPtr, UTF8Content.Length());

		/**
			PKCS5Padding
		*/
		int32 Padding = FAES::AESBlockSize - (UTF8Content.Length() % FAES::AESBlockSize);

		TArray<uint8> PaddingBlock;
		PaddingBlock.SetNum(Padding);
		FMemory::Memset(PaddingBlock.GetData(), (uint8)Padding, Padding);
		Data.Append(PaddingBlock);

		FAES::EncryptData(Data.GetData(), Data.Num(), SecretKey.GetData(), SecretKey.Num());
		Encrypt = FBase64::Encode(Data);
	}

	FString DecryptData(const FString& EncryptContent)
	{
		TArray<uint8> DecodeContent;
		FBase64::Decode(EncryptContent, DecodeContent);
		FAES::DecryptData(DecodeContent.GetData(), DecodeContent.Num(), SecretKey.GetData(), SecretKey.Num());

		uint8 Padding = DecodeContent.Last();
		DecodeContent.RemoveAt(DecodeContent.Num() - Padding, Padding);
		uint8* ContentData = DecodeContent.GetData();
		*(ContentData + DecodeContent.Num()) = '\0';

		FString DecryptText;
		TArray<TCHAR>& Chars = DecryptText.GetCharArray();
		FUTF8ToTCHAR TChar((ANSICHAR*)ContentData);
		Chars.Append(TChar.Get(), TChar.Length());
		return DecryptText;
	}

private:

	TArray<uint8> SecretKey;
};