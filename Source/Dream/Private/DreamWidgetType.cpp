// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamWidgetType.h"


#include "DMProjectSettings.h"
#include "DMUpgradeAddition.h"
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

EPropsQuality UWItem::GetQuality() const
{
	TSharedPtr<FItem> Item = GetItem();

	if (Item.IsValid())
	{
		return ItemUtils::GetItemQuality(Item->GetItemGuid());
	}

	return EPropsQuality::Normal;
}

TEnumAsByte<EItemType::Type> UWItem::GetItemType() const
{
	TSharedPtr<FItem> Item = GetItem();
	if (Item.IsValid())
	{
		return ItemUtils::GetItemType(Item->GetItemGuid());
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

int32 UWItemEquipment::GetGearLevel() const
{
	return ItemEquipment->GearLevel;
}

void UWItemEquipment::IncreaseLevel()
{
	ItemEquipment->GearLevel++;
}

float UWItemEquipment::GetAttributeMagnitude(FAttributeHandle Attribute) const
{
	FFloatProperty* Property = Attribute.AttributeProperty.Get();
	if (Property && ItemEquipment.IsValid())
	{
		return Property->GetPropertyValue_InContainer(&ItemEquipment->Attributes);
	}

	return 0.f;
}

void UWItemEquipment::GetGearUpgradeAdditionMagnitude(FAttributeHandle Attribute, float& AttributeMagnitude, float& AdditionMagnitude)
{
	AttributeMagnitude = 0.f;
	AdditionMagnitude = 0.f;
	
	if (ItemEquipment.IsValid())
	{
		if (FFloatProperty* Property = Attribute.AttributeProperty.Get())
		{
			AttributeMagnitude = Property->GetPropertyValue_InContainer(&ItemEquipment->Attributes);

			if (UpgradeGearInfluence && UpgradeGearInfluence->IsInfluence(Attribute) && ItemEquipment->GearLevel > 0)
			{
				EPropsQuality Quality = ItemUtils::GetItemQuality(ItemEquipment->GetItemGuid());
				UDMUpgradeAddition* UpgradeAddition = GSProject->GetUpgradeAddition();
				float AdditionStrength = UpgradeAddition->GetAdditionStrength(Quality, ItemEquipment->GearLevel);
				AdditionMagnitude = AttributeMagnitude * AdditionStrength;
			}
		}
	}
}

void UWItemEquipment::SetItemEquipment(TSharedPtr<FItemEquipment> InItem)
{
	ItemEquipment = InItem;

	if (UClass* ItemClass = GetItemClass())
	{
		if (IGearInterface* GearInterface = Cast<IGearInterface>(ItemClass->GetDefaultObject()))
		{
			UpgradeGearInfluence = GearInterface->GetUpgradeAttributesInfluence();
		}
	}
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
