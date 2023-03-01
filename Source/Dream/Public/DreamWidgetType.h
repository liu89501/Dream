// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "DreamWidgetType.generated.h"

class UDMUpgradeGearInfluence;

UENUM(BlueprintType)
enum class EInteractiveType : uint8
{
	DecomposeEquipment
};


UCLASS(Abstract, BlueprintType)
class UWItem : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category=ViewData)
	int64 GetItemID() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
	int32 GetItemGuid() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
    UClass* GetItemClass() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
    const FPropsInfo& GetPropsInfo() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
	EPropsQuality GetQuality() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
    TEnumAsByte<EItemType::Type> GetItemType() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
    FItemListHandle MakeItemListHandle() const;

public:

	virtual TSharedPtr<FItem> GetItem() const;

	virtual int64 NativeGetItemID() const;
	
};

UCLASS()
class UWItemSimple : public UWItem
{
	GENERATED_BODY()

public:

	void SetItem(TSharedPtr<FItemSimple> InItem)
	{
		ItemSimple = InItem;
	}

	virtual int64 NativeGetItemID() const override
	{
		return ItemId;
	}

	virtual TSharedPtr<FItem> GetItem() const override
	{
		return ItemSimple;
	}

public:

	UPROPERTY(BlueprintReadOnly, Category=ViewData, Meta = (ExposeOnSpawn))
	int64 ItemId;

private:

	TSharedPtr<FItemSimple> ItemSimple;

};


UCLASS()
class UWItemEquipment : public UWItem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category=ViewData)
	const FEquipmentAttributes& GetAttributes() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
	int32 GetGearLevel() const;

	UFUNCTION(BlueprintCallable, Category=ViewData)
	void IncreaseLevel();

	UFUNCTION(BlueprintCallable, Category=ViewData)
	float GetAttributeMagnitude(FAttributeHandle Attribute) const;

	UFUNCTION(BlueprintPure, Category=ViewData)
	void GetGearUpgradeAdditionMagnitude(FAttributeHandle Attribute, float& AttributeMagnitude, float& AdditionMagnitude);

	virtual TSharedPtr<FItem> GetItem() const override
	{
		return ItemEquipment;
	}

	virtual int64 NativeGetItemID() const override
	{
		return EquipmentId;
	}

	void SetItemEquipment(TSharedPtr<FItemEquipment> InItem);

public:

	UPROPERTY(BlueprintReadOnly, Category=ViewData, Meta = (ExposeOnSpawn))
	int64 EquipmentId;

private:

	UPROPERTY()
	UDMUpgradeGearInfluence* UpgradeGearInfluence;

	TSharedPtr<FItemEquipment> ItemEquipment;
};


UCLASS(BlueprintType)
class UTaskViewData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category=ViewData, Meta = (ExposeOnSpawn))
	FTaskInformation Task;

	UFUNCTION(BlueprintCallable, Category=ViewData)
	void ToggleTaskTracking();
};


UCLASS(BlueprintType)
class UWStoreItem : public UObject
{
	GENERATED_BODY()

public:

	void SetStoreItem(TSharedPtr<FItem> Item);

	void Initialize(const FStoreItem& StoreItem);

private:

	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess="true"), Category=StoreItem)
	UWItem* ActualItem;

	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess="true"), Category=StoreItem)
	TArray<FAcquisitionCost> Costs;

	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess="true"), Category=StoreItem)
	int64 StoreItemID;
};

USTRUCT(BlueprintType)
struct FWStoreInformation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category=Store)
	int32 TotalPage;

	UPROPERTY(BlueprintReadOnly, Category=Store)
	int32 TotalItems;

	UPROPERTY(BlueprintReadOnly, Category=Store)
	TArray<UWStoreItem*> Items;
};

