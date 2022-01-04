// Fill out your copyright notice in the Description page of Project Settings.


#include "DProjectSettings.h"

#include "DamageWidgetComponent.h"
#include "SurfaceImpactAsset.h"

FSurfaceImpactEffect InvalidImpactEffect = FSurfaceImpactEffect();
FQualityInfo InvalidQualityInfo = FQualityInfo();

FText UDProjectSettings::GetWeaponTypeName(EWeaponType WeaponType) const
{
	if (const FText* NameText = WeaponTypeName.Find(WeaponType))
	{
		return *NameText;
	}

	return FText::FromName(NAME_None);
}

FText UDProjectSettings::GetWeaponFireMode(EFireMode FireMode) const
{
	if (const FText* NameText = FireModeNameSettings.Find(FireMode))
	{
		return *NameText;
	}

	return FText::FromName(NAME_None);
}

FText UDProjectSettings::GetItemTypeName(TEnumAsByte<EItemType::Type> ItemType) const
{
	if (const FText* NameText = ItemNameSettings.Find(ItemType))
	{
		return *NameText;
	}

	return FText::FromName(NAME_None);
}

const FQualityInfo& UDProjectSettings::GetQualityInfo(EPropsQuality Quality) const
{
	const FQualityInfo* QualityInfo = QualitySettings.Find(Quality);
	return QualityInfo ? *QualityInfo : InvalidQualityInfo;
}

const FSurfaceImpactEffect& UDProjectSettings::GetSurfaceImpactEffect(EPhysicalSurface SurfaceType) const
{
	if (USurfaceImpactAsset* SurfaceImpact = GetSurfaceImpactAsset())
	{
		const FSurfaceImpactEffect* EffectPtr = SurfaceImpact->SurfaceImpactEffects.Find(SurfaceType);
		return EffectPtr ? *EffectPtr : InvalidImpactEffect;
	}
	
	return InvalidImpactEffect;
}

ULevelListAsset* UDProjectSettings::GetLevelListAsset() const
{
	UObject* ResolveObject = MainInterfaceWorld.ResolveObject();
	if (ResolveObject == nullptr)
	{
		ResolveObject = MainInterfaceWorld.TryLoad();
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

const FItemDefinition& UDProjectSettings::GetItemDefinition(int32 ItemGuid) const
{
	if (UDataTable* DataTable = Cast<UDataTable>(ItemsTable.TryLoad()))
	{
		if (uint8* Unchecked = DataTable->FindRowUnchecked(FName(FString::FromInt(ItemGuid))))
		{
			return *reinterpret_cast<FItemDefinition*>(Unchecked);
		}
	}

	return FEmptyStruct::EmptyItemDefinition;
}

UClass* UDProjectSettings::GetItemClassFromGuid(int32 ItemGuid) const
{
	const FItemDefinition& Definition = GetItemDefinition(ItemGuid);
	return LoadClass<UObject>(GetTransientPackage(), *Definition.ItemClass);
}

bool UDProjectSettings::GetAllItems(EItemType::Type ItemType, TArray<FItemDetails>& Items) const
{
	if (UDataTable* DataTable = Cast<UDataTable>(ItemsTable.TryLoad()))
	{
		for (TTuple<FName, uint8*> Row : DataTable->GetRowMap())
		{
			int32 ItemGuid = FCString::Atoi(*Row.Key.ToString());
			EItemType::Type Type = GetItemType(ItemGuid);

			if (Type == ItemType || ItemType == EItemType::All)
			{
				FItemDefinition* Def = reinterpret_cast<FItemDefinition*>(Row.Value);
				FItemDetails Details;
				Details.ItemClass = Def->ItemClass;
				Details.ItemGuid = ItemGuid;
				Items.Add(Details);
			}
		}

		return true;
	}

	return false;
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

