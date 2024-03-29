// Fill out your copyright notice in the Description page of Project Settings.

#include "DMProjectSettings.h"
#include "ShootWeapon.h"
#include "DamageWidgetComponent.h"
#include "DBaseAttributesAsset.h"
#include "DMMantleAsset.h"
#include "DMRollingAsset.h"
#include "DMUISoundSet.h"
#include "DMUpgradeAddition.h"
#include "GameMapsSettings.h"
#include "PlayerDataInterfaceType.h"
#include "SurfaceImpactAsset.h"
#include "Curves/CurveVector.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/KismetMathLibrary.h"

#define DM_LOAD_CLASS(ClassName, SoftClass, Assign) \
	if (SoftClass.IsValid()) \
	{ \
		Assign = SoftClass.TryLoadClass<ClassName>();\
	} \
	else \
	{ \
		Assign = nullptr; \
	}

#define DM_LOAD_OBJECT(ClassName, SoftObject, Assign) \
	if (SoftObject.IsValid()) \
	{ \
		Assign = Cast<ClassName>(SoftObject.TryLoad());\
	} \
	else \
	{ \
		Assign = nullptr; \
	}

#define Lazy_Load_Class(c, sc, v) \
	if (v == nullptr) \
	{ \
		DM_LOAD_CLASS(c, UDMProjectSettings::Get()->sc, v) \
	}

#define Lazy_Load_Object(c, sc, v) \
	if (v == nullptr) \
	{ \
		DM_LOAD_OBJECT(c, UDMProjectSettings::Get()->sc, v) \
	}

FSurfaceImpactEffect InvalidImpactEffect = FSurfaceImpactEffect();
FQualityInfo InvalidQualityInfo = FQualityInfo();
FItemTypeSettingsInfo InvalidItemTypeInfo = FItemTypeSettingsInfo(NAME_None);
FText TEXT_None = FText::FromName(NAME_None);

DREAM_API UDMProjectSettingsInstance* GSProject = nullptr;

UDMProjectSettings::UDMProjectSettings()
	: CombatToIdleTime(1.f)
	, PickupAmmunitionAmount(FRangeRandomFloat(0.1f, 0.25f))
{
	
}

const FText& UDMProjectSettings::GetWeaponTypeName(EWeaponType WeaponType) const
{
	switch (WeaponType)
	{
	case EWeaponType::AssaultRifle:		return WeaponTypeDisplayNames.AssaultRifle;
	case EWeaponType::Shotgun:			return WeaponTypeDisplayNames.Shotgun;
	case EWeaponType::GrenadeLaunch:	return WeaponTypeDisplayNames.GrenadeLaunch;
	case EWeaponType::PrecisionRifle:	return WeaponTypeDisplayNames.PrecisionRifle;
	case EWeaponType::SniperRifle:		return WeaponTypeDisplayNames.SniperRifle;
	}

	return TEXT_None;
}

const FText& UDMProjectSettings::GetWeaponFireMode(EFireMode FireMode) const
{
	switch (FireMode)
	{
	case EFireMode::Accumulation:	return FireModeDisplayNames.Accumulation;
	case EFireMode::FullyAutomatic: return FireModeDisplayNames.FullyAutomatic;
	case EFireMode::SemiAutomatic:	return FireModeDisplayNames.SemiAutomatic;
	}

	return TEXT_None;
}

const FText& UDMProjectSettings::GetItemTypeName(TEnumAsByte<EItemType::Type> ItemType) const
{
	return GetItemTypeInfo(ItemType).ItemTypeDisplayName;
}

const FQualityInfo& UDMProjectSettings::GetQualityInfo(EPropsQuality Quality) const
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

const FSurfaceImpactEffect& UDMProjectSettingsInstance::GetSurfaceImpactEffect(EPhysicalSurface SurfaceType)
{
	if (USurfaceImpactAsset* SurfaceImpact = GetSurfaceImpactAsset())
	{
		return SurfaceImpact->GetImpactEffect(SurfaceType);
	}
	
	return InvalidImpactEffect;
}


const FName& UDMProjectSettings::GetWepActiveSockName() const
{
	return WeaponActivateSocketName;
}

const FName& UDMProjectSettings::GetWepHolsterSockName() const
{
	return WeaponHolsterSocketName;
}

const FSoftObjectPath& UDMProjectSettings::GetMainUILevel() const
{
	return MainInterfaceWorld;
}

FString UDMProjectSettings::GetMainUILevelAsString() const
{
	return MainInterfaceWorld.GetLongPackageName();
}

float UDMProjectSettings::GetPickupAmmunitionAmount() const
{
	return UKismetMathLibrary::RandomFloatInRange(PickupAmmunitionAmount.Min, PickupAmmunitionAmount.Max);
}

float UDMProjectSettings::GetIdleSwitchingTime() const
{
	return CombatToIdleTime;
}

const FItemTypeSettingsInfo& UDMProjectSettings::GetItemTypeInfo(EItemType::Type ItemType) const
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

const FMantleTraceSettings& UDMProjectSettings::GetMantleTraceSettings() const
{
	return MantleTraceSettings;
}

TSubclassOf<ADDropReward> UDMProjectSettings::GetRewardDropClass(EItemType::Type ItemType) const
{
	return GetItemTypeInfo(ItemType).DropClass;
}

TSubclassOf<ADDropMagazine> UDMProjectSettings::GetMagazineDropClass(EAmmoType AmmoType) const
{
	switch (AmmoType)
	{
	case EAmmoType::Level1: return MagazineDropSettings.AmmoL1;
	case EAmmoType::Level2:	return MagazineDropSettings.AmmoL2;
	case EAmmoType::Level3: return MagazineDropSettings.AmmoL3;
	}
	
	return nullptr;
}

UDMProjectSettings* UDMProjectSettings::GetProjectSettings()
{
	return GetMutableDefault<UDMProjectSettings>();
}

const UDMProjectSettings* UDMProjectSettings::Get()
{
	return GetDefault<UDMProjectSettings>();
}

#if WITH_EDITOR

void UDMProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
	{
		if (GSProject)
		{
			GSProject->InitializeSettingsInstances();
		}
	}
}

#endif


FBaseAttributes UDMProjectSettingsInstance::GetBaseAttributes(int32 Level) const
{
	if (PlayerBaseAttributes)
	{
		return PlayerBaseAttributes->BaseAttributes * (1 + Level * PlayerBaseAttributes->IncrementPerLevel);
	}

	return FBaseAttributes();
}

const FItemDef& UDMProjectSettingsInstance::GetItemDefinition(int32 ItemGuid) const
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

UClass* UDMProjectSettingsInstance::GetItemClassFromGuid(int32 ItemGuid) const
{
	const FItemDef& Definition = GetItemDefinition(ItemGuid);
	return Definition.GetItemClass();
}

void UDMProjectSettingsInstance::GetTalents(ETalentCategory Category, int64 Talents, TArray<FTalentInfo>& AllTalents) const
{
	if (TalentAsset)
	{
		switch (Category)
		{
		case ETalentCategory::Warrior: AllTalents = TalentAsset->Talent_Warrior; break;
		case ETalentCategory::SniperExpert: AllTalents = TalentAsset->Talent_SniperExpert; break;
		case ETalentCategory::BlastingGrandmaster: AllTalents = TalentAsset->Talent_BlastingGrandmaster; break;
		}

		for (FTalentInfo& Info : AllTalents)
		{
			Info.bLearned = ((Talents >> Info.TalentIndex) & 1) == 1;
		}
	}
}

void UDMProjectSettingsInstance::GetLearnedTalents(ETalentCategory Category, int64 Talents, TArray<FTalentInfo>& LearnedTalents) const
{
	if (TalentAsset)
	{
		const TArray<FTalentInfo>& TalentBranch = Category == ETalentCategory::Warrior ? TalentAsset->Talent_Warrior :
			Category == ETalentCategory::SniperExpert ? TalentAsset->Talent_SniperExpert : TalentAsset->Talent_BlastingGrandmaster;

		for (const FTalentInfo& Info : TalentBranch)
		{
			if (((Talents >> Info.TalentIndex) & 1) == 1)
			{
				FTalentInfo CopyTalent = Info;
				CopyTalent.bLearned = true;
				LearnedTalents.Add(CopyTalent);
			}
		}
	}
}

UDataTable* UDMProjectSettingsInstance::GetItemTable() const
{
	return ItemListTable;
}

void UDMProjectSettingsInstance::Initialize()
{
	InitializeSettingsInstances();
	AddToRoot();
}

void UDMProjectSettingsInstance::FinishDestroy()
{
	Super::FinishDestroy();
	RemoveFromRoot();
}

UClass* UDMProjectSettingsInstance::GetMasterAnimClass()
{
	Lazy_Load_Class(UAnimInstance, PawnMasterAnimClass, PawnMasterAnimClass);
	return PawnMasterAnimClass;
}

UClass* UDMProjectSettingsInstance::GetSlaveAnimClass()
{
	Lazy_Load_Class(UAnimInstance, PawnSlaveAnimClass, PawnSlaveAnimClass);
	return PawnSlaveAnimClass;
}

UClass* UDMProjectSettingsInstance::GetDamageWidgetClass()
{
	Lazy_Load_Class(UDamageWidgetComponent, DamageWidgetClass, DamageWidgetClass);
	return DamageWidgetClass;
}

ULevelListAsset* UDMProjectSettingsInstance::GetLevelListAsset()
{
	Lazy_Load_Object(ULevelListAsset, LevelAsset, LevelListAsset);
	return LevelListAsset;
}

USurfaceImpactAsset* UDMProjectSettingsInstance::GetSurfaceImpactAsset()
{
	Lazy_Load_Object(USurfaceImpactAsset, SurfaceImpactAsset, SurfaceImpactAsset);
	return SurfaceImpactAsset;
}

UTalentAsset* UDMProjectSettingsInstance::GetTalentAsset() const
{
	return TalentAsset;
}

UDBaseAttributesAsset* UDMProjectSettingsInstance::GetPlayerBaseAttributes()
{
	Lazy_Load_Object(UDBaseAttributesAsset, PlayerBaseAttributes, PlayerBaseAttributes);
	return PlayerBaseAttributes;
}

UCurveFloat* UDMProjectSettingsInstance::GetRotationRateCurve()
{
	Lazy_Load_Object(UCurveFloat, RotationRateCurve, RotationRateCurve);
	return RotationRateCurve;
}

UCurveVector* UDMProjectSettingsInstance::GetMovementCurve()
{
	Lazy_Load_Object(UCurveVector, MovementCurve, MovementCurve);
	return MovementCurve;
}

UCurveFloat* UDMProjectSettingsInstance::GetMantlePositionCurve()
{
	Lazy_Load_Object(UCurveFloat, MantlePositionCurve, MantlePositionCurve);
	return MantlePositionCurve;
}

UCurveFloat* UDMProjectSettingsInstance::GetMantlingRotationCurve()
{
	Lazy_Load_Object(UCurveFloat, MantlingRotationCurve, MantlingRotationCurve);
	return MantlingRotationCurve;
}

UDMMantleAsset* UDMProjectSettingsInstance::GetMantleAsset()
{
	Lazy_Load_Object(UDMMantleAsset, MantleAsset, MantleAsset);
	return MantleAsset;
}

UDMUISoundSet* UDMProjectSettingsInstance::GetUISoundSet()
{
	Lazy_Load_Object(UDMUISoundSet, UISoundSetAsset, UISoundSet);
	return UISoundSet;
}

UDMUpgradeAddition* UDMProjectSettingsInstance::GetUpgradeAddition()
{
	Lazy_Load_Object(UDMUpgradeAddition, UpgradeAdditionAsset, UpgradeAddition);
	return UpgradeAddition;
}

UDMRollingAsset* UDMProjectSettingsInstance::GetRollingAsset()
{
	Lazy_Load_Object(UDMRollingAsset, RollingAsset, RollingAsset);
	return RollingAsset;
}

const FMantleInformation& UDMProjectSettingsInstance::GetMantleInfo(EMantleType MantleType, EOverlayState OverlayState)
{
	return GetMantleAsset()->GetMantleInfo(MantleType, OverlayState);
}

void UDMProjectSettingsInstance::InitializeSettingsInstances()
{
	UDMProjectSettings* ProjectSettings = GetMutableDefault<UDMProjectSettings>();
	
	DM_LOAD_OBJECT(UDataTable, ProjectSettings->ItemsTable, ItemListTable);
	DM_LOAD_OBJECT(UTalentAsset, ProjectSettings->TalentAsset, TalentAsset);
}
