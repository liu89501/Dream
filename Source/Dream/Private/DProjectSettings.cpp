// Fill out your copyright notice in the Description page of Project Settings.

#include "DProjectSettings.h"
#include "ShootWeapon.h"
#include "DamageWidgetComponent.h"
#include "PlayerDataInterfaceType.h"
#include "SurfaceImpactAsset.h"
#include "Kismet/KismetMathLibrary.h"

FSurfaceImpactEffect InvalidImpactEffect = FSurfaceImpactEffect();
FQualityInfo InvalidQualityInfo = FQualityInfo();
FItemTypeSettingsInfo InvalidItemTypeInfo = FItemTypeSettingsInfo(NAME_None);

UDProjectSettings::UDProjectSettings()
	: CombatToIdleTime(1.f)
	, PickupAmmunitionAmount(FRangeRandomFloat(0.1f, 0.25f))
{
	
}

FText UDProjectSettings::GetWeaponTypeName(EWeaponType WeaponType) const
{
	switch (WeaponType)
	{
		case EWeaponType::AssaultRifle:		return WeaponTypeDisplayNames.AssaultRifle;
		case EWeaponType::Shotgun:			return WeaponTypeDisplayNames.Shotgun;
		case EWeaponType::GrenadeLaunch:	return WeaponTypeDisplayNames.GrenadeLaunch;
		case EWeaponType::PrecisionRifle:	return WeaponTypeDisplayNames.PrecisionRifle;
		case EWeaponType::SniperRifle:		return WeaponTypeDisplayNames.SniperRifle;
	}

	return FText::FromName(NAME_None);
}

FText UDProjectSettings::GetWeaponFireMode(EFireMode FireMode) const
{
	switch (FireMode)
	{
		case EFireMode::Accumulation:	return FireModeDisplayNames.Accumulation;
		case EFireMode::FullyAutomatic: return FireModeDisplayNames.FullyAutomatic;
		case EFireMode::SemiAutomatic:	return FireModeDisplayNames.SemiAutomatic;
	}

	return FText::FromName(NAME_None);
}

FText UDProjectSettings::GetItemTypeName(TEnumAsByte<EItemType::Type> ItemType) const
{
	return GetItemTypeInfo(ItemType).ItemTypeDisplayName;
}

const FQualityInfo& UDProjectSettings::GetQualityInfo(EPropsQuality Quality) const
{
	switch (Quality)
	{
		case EPropsQuality::Normal:		return ItemQualities.Normal;
		case EPropsQuality::Advanced:	return ItemQualities.Advanced;
	    case EPropsQuality::Rare:		return ItemQualities.Rare;
	    case EPropsQuality::Epic:		return ItemQualities.Epic;
	    case EPropsQuality::Legendary:	return ItemQualities.Legendary;
	}
	
	return InvalidQualityInfo;
}

const FSurfaceImpactEffect& UDProjectSettings::GetSurfaceImpactEffect(EPhysicalSurface SurfaceType) const
{
	if (USurfaceImpactAsset* SurfaceImpact = GetSurfaceImpactAsset())
	{
		return SurfaceImpact->GetImpactEffect(SurfaceType);
	}
	
	return InvalidImpactEffect;
}

ULevelListAsset* UDProjectSettings::GetLevelListAsset() const
{
	UObject* ResolveObject = LevelAsset.ResolveObject();
	if (ResolveObject == nullptr)
	{
		ResolveObject = LevelAsset.TryLoad();
	}
	
	return Cast<ULevelListAsset>(ResolveObject);
}

const FName& UDProjectSettings::GetWepActiveSockName() const
{
	return WeaponActivateSocketName;
}

const FName& UDProjectSettings::GetWepHolsterSockName() const
{
	return WeaponHolsterSocketName;
}

const FSoftObjectPath& UDProjectSettings::GetMainUILevel() const
{
	return MainInterfaceWorld;
}

FString UDProjectSettings::GetMainUILevelAsString() const
{
	return MainInterfaceWorld.GetLongPackageName();
}

USurfaceImpactAsset* UDProjectSettings::GetSurfaceImpactAsset() const
{
	UObject* Object = SurfaceImpactAsset.ResolveObject();
	if (Object == nullptr)
	{
		Object = SurfaceImpactAsset.TryLoad();
	}
	return Cast<USurfaceImpactAsset>(Object);
}

UClass* UDProjectSettings::GetDamageComponentClass() const
{
	UClass* ComponentClass = DamageWidgetClass.ResolveClass();
	if (ComponentClass == nullptr)
	{
		ComponentClass = DamageWidgetClass.TryLoadClass<UDamageWidgetComponent>();
	}
	return ComponentClass;
}

float UDProjectSettings::GetPickupAmmunitionAmount() const
{
	return UKismetMathLibrary::RandomFloatInRange(PickupAmmunitionAmount.Min, PickupAmmunitionAmount.Max);
}

float UDProjectSettings::GetIdleSwitchingTime() const
{
	return CombatToIdleTime;
}

const FItemTypeSettingsInfo& UDProjectSettings::GetItemTypeInfo(EItemType::Type ItemType) const
{
	switch (ItemType)
	{
		case EItemType::Weapon:		return ItemTypeSettings.Weapon;
		case EItemType::Module:		return ItemTypeSettings.Module;
		case EItemType::Experience:	return ItemTypeSettings.Experience;
		case EItemType::Material:	return ItemTypeSettings.Material;
		case EItemType::Consumable:	return ItemTypeSettings.Consumable;
		case EItemType::Ability:	return ItemTypeSettings.Ability;
		default:					return InvalidItemTypeInfo;
	}
}

TSubclassOf<ADDropReward> UDProjectSettings::GetRewardDropClass(EItemType::Type ItemType)
{
	return GetItemTypeInfo(ItemType).DropClass;
}

TSubclassOf<ADDropMagazine> UDProjectSettings::GetMagazineDropClass(EAmmoType AmmoType)
{
	switch (AmmoType)
	{
		case EAmmoType::Level1: return MagazineDropSettings.AmmoL1;
		case EAmmoType::Level2:	return MagazineDropSettings.AmmoL2;
		case EAmmoType::Level3: return MagazineDropSettings.AmmoL3;
	}
	
	return nullptr;
}

UDataTable* UDProjectSettings::GetItemTable() const
{
	return Cast<UDataTable>(ItemsTable.TryLoad());
}

UDProjectSettings* UDProjectSettings::GetProjectSettings()
{
	return GetMutableDefault<UDProjectSettings>();
}

UClass* UDProjectSettings::GetMasterAnimClass() const
{
	UClass* AnimClass = PawnMasterAnimClass.ResolveClass();
	if (AnimClass == nullptr)
	{
		AnimClass = PawnMasterAnimClass.TryLoadClass<UAnimInstance>();
	}
	return AnimClass;
}

UClass* UDProjectSettings::GetSlaveAnimClass() const
{
	UClass* AnimClass = PawnSlaveAnimClass.ResolveClass();
	if (AnimClass == nullptr)
	{
		AnimClass = PawnSlaveAnimClass.TryLoadClass<UAnimInstance>();
	}
	return AnimClass;
}

const FItemDef& UDProjectSettings::GetItemDefinition(int32 ItemGuid) const
{
	if (UDataTable* DataTable = GetItemTable())
	{
		if (uint8* Unchecked = DataTable->FindRowUnchecked(FName(FString::FromInt(ItemGuid))))
		{
			return *reinterpret_cast<FItemDef*>(Unchecked);
		}
	}

	return FEmptyStruct::EmptyItemDef;
}

UClass* UDProjectSettings::GetItemClassFromGuid(int32 ItemGuid) const
{
	const FItemDef& Definition = GetItemDefinition(ItemGuid);
	return Definition.ItemClass.TryLoadClass<UObject>();
}

TArray<FTalentInfo> UDProjectSettings::GetTalents(ETalentCategory Category, int64 Talents) const
{
	TArray<FTalentInfo> AllTalents;
	
	if (UTalentAsset* TAsset = Cast<UTalentAsset>(TalentAsset.TryLoad()))
	{
		switch (Category)
		{
		case ETalentCategory::Warrior: AllTalents = TAsset->Talent_Warrior; break;
		case ETalentCategory::SniperExpert: AllTalents = TAsset->Talent_SniperExpert; break;
		case ETalentCategory::BlastingGrandmaster: AllTalents = TAsset->Talent_BlastingGrandmaster; break;
		}

		for (FTalentInfo& Info : AllTalents)
		{
			Info.bLearned = ((Talents >> Info.TalentIndex) & 1) == 1;
		}
	}
	return AllTalents;
}

TArray<FTalentInfo> UDProjectSettings::GetLearnedTalents(ETalentCategory Category, int64 LearnedTalents) const
{
	TArray<FTalentInfo> LearnedTalentArray;

	if (UTalentAsset* TAsset = Cast<UTalentAsset>(TalentAsset.TryLoad()))
	{
		const TArray<FTalentInfo>& TalentBranch = Category == ETalentCategory::Warrior ? TAsset->Talent_Warrior :
			Category == ETalentCategory::SniperExpert ? TAsset->Talent_SniperExpert : TAsset->Talent_BlastingGrandmaster;

		for (const FTalentInfo& Info : TalentBranch)
		{
			if (((LearnedTalents >> Info.TalentIndex) & 1) == 1)
			{
				FTalentInfo CopyTalent = Info;
				CopyTalent.bLearned = true;
				LearnedTalentArray.Add(CopyTalent);
			}
		}
	}

	return LearnedTalentArray;
}

