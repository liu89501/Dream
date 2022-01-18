// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamWidgetType.h"
#include "DreamWidgetStatics.h"

int64 UWItem::GetItemID() const
{
	return NativeGetItemID();
}

int32 UWItem::GetItemGuid() const
{
	TSharedPtr<FItem> Item = GetItem();

	if (Item.IsValid())
	{
		return Item->GetItemGuid();
	}
	return 0;
}

UClass* UWItem::GetItemClass() const
{
	TSharedPtr<FItem> Item = GetItem();
	if (Item.IsValid())
	{
		return UDreamWidgetStatics::GetItemClassByGuid(Item->GetItemGuid());
	}
	return nullptr;
}

const FPropsInfo& UWItem::GetPropsInfo() const
{
	TSharedPtr<FItem> Item = GetItem();
	if (Item.IsValid())
	{
		return UDreamWidgetStatics::GetPropsInfoByItemGuid(Item->GetItemGuid());
	}
	return FEmptyStruct::EmptyPropsInfo;
}

TEnumAsByte<EItemType::Type> UWItem::GetItemType() const
{
	TSharedPtr<FItem> Item = GetItem();
	if (Item.IsValid())
	{
		return ::GetItemType(Item->GetItemGuid());
	}
	return EItemType::INVALID;
}

FItemListHandle UWItem::MakeItemListHandle() const
{
	FItemListHandle Handle;
	Handle.AddItem(GetItem());
	return Handle;
}

TSharedPtr<FItem> UWItem::GetItem() const
{
	return nullptr;
}

int64 UWItem::NativeGetItemID() const
{
	return 0;
}

const FEquipmentAttributes& UWItemEquipment::GetAttributes() const
{
	return ItemEquipment->Attributes;
}

void UTaskViewData::ToggleTaskTracking()
{
	Task.bTracking = !Task.bTracking;
}

void UWStoreItem::SetStoreItem(TSharedPtr<FItem> Item)
{
	if (Item.IsValid())
	{
		uint8 StructMark = Item->GetStructMark();

		switch (StructMark)
		{
		case IM_Equipment:
			{
				UWItemEquipment* ItemEquipment = NewObject<UWItemEquipment>(this);
				ItemEquipment->SetItemEquipment(StaticCastSharedPtr<FItemEquipment>(Item));
				ActualItem = ItemEquipment;
			}
			break;
		case IM_Simple:
			{
				UWItemSimple* ItemSimple = NewObject<UWItemSimple>(this);
				ItemSimple->SetItem(StaticCastSharedPtr<FItemSimple>(Item));
				ActualItem = ItemSimple;
			}
			break;
		default:
			ActualItem = nullptr;
		}
	}
}

void UWStoreItem::Initialize(const FStoreItem& StoreItem)
{
	SetStoreItem(StoreItem.Item);
	StoreItemID = StoreItem.SIID;
	Costs = StoreItem.Costs;
}
