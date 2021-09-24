// Fill out your copyright notice in the Description page of Project Settings.


#include "PDI/PlayerDataInterfaceType.h"


#include "DExperience.h"
#include "DGameplayStatics.h"
#include "DModuleBase.h"
#include "DMoney.h"
#include "ShootWeapon.h"

void UItemData::PostInitProperties()
{
	Super::PostInitProperties();
	InitializeProperties();
}

void UItemData::InitializeProperties()
{
}

UClass* UItemData::GetItemClass() const
{
	return nullptr;
}

int32 UItemData::GetItemAmount() const
{
	return 1;
}

EItemType::Type UItemData::NativeGetItemType() const
{
	return EItemType::INVALID;
}

const FPropsInfo& UItemData::GetPropsInformation() const
{
	if (UClass* ItemClass = GetItemClass())
	{
		if (IPropsInterface* PropsInterface = Cast<IPropsInterface>(ItemClass->GetDefaultObject()))
		{
			return PropsInterface->GetPropsInfo();
		}
	}

	return FEmptyStruct::EmptyPropsInfo;
}

TEnumAsByte<EItemType::Type> UItemData::GetItemType() const
{
	return NativeGetItemType();
}

void UItemDataWeapon::InitializeProperties()
{
	if (!WeaponClass.IsValid())
	{
		return;
	}

	DREAM_NLOG(Error, TEXT("POst UItemDataWeapon PostInitProperties"));
	
	AShootWeapon* CDO = GetItemClass()->GetDefaultObject<AShootWeapon>();

	RateOfFire = CDO->RateOfFire;
	Magazine = CDO->AmmoNum;
	FireMode = UEnum::GetDisplayValueAsText(CDO->FireMode);
}

UClass* UItemDataWeapon::GetItemClass() const
{
	UClass* Class = WeaponClass.TryLoadClass<AShootWeapon>();
	checkf(Class, TEXT("WeaponClass Invalid"));
	return Class;
}

void UItemDataWeapon::AttemptAssignAttributes()
{
	if (!bFixedAttributes)
	{
		AShootWeapon* WeaponCDO = GetItemClass()->GetDefaultObject<AShootWeapon>();
		Attributes = FEquipmentAttributesAssign::AssignAttributes(WeaponCDO->AttributesAssign);
	}
}

void UItemDataModule::InitializeProperties()
{
	if (!ModuleClass.IsValid())
	{
		return;
	}

	DREAM_NLOG(Error, TEXT("POst UItemDataModule PostInitProperties"));
	
	UDModuleBase* ModuleCDO = GetItemClass()->GetDefaultObject<UDModuleBase>();
	Category = ModuleCDO->Category;
}

UClass* UItemDataModule::GetItemClass() const
{
	UClass* Class = ModuleClass.TryLoadClass<UDModuleBase>();
	checkf(Class, TEXT("ModuleClass Invalid"));
	return Class;
}

void UItemDataModule::AttemptAssignAttributes()
{
	if (!bFixedAttributes)
	{
		UDModuleBase* ModuleCDO = GetItemClass()->GetDefaultObject<UDModuleBase>();
		Attributes = FEquipmentAttributesAssign::AssignAttributes(ModuleCDO->AttributesAssign);
	}
}

UClass* UItemDataExperience::GetItemClass() const
{
	return UDExperience::StaticClass();
}

UClass* UItemDataMoney::GetItemClass() const
{
	return UDMoney::StaticClass();
}
