// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DDropMagazine.h"
#include "DPropsType.h"
#include "DreamType.h"
#include "UObject/NoExportTypes.h"
#include "DProjectSettings.generated.h"

class ADDropReward;

struct FTalentInfo;

enum class EWeaponType : uint8;
enum class EFireMode : uint8;
enum class ETalentCategory : uint8;
enum class EAmmoType : uint8;


USTRUCT(BlueprintType)
struct FLevelInformation
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "World"), BlueprintReadOnly)
	FSoftObjectPath Map;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString GameModeClassAlias;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* MapIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

public:

	int32 GetGameModeMaxPlayers() const;
};

USTRUCT()
struct FItemTypeSettingsInfo
{
	GENERATED_BODY()

	FItemTypeSettingsInfo()
		: DropClass(nullptr)
	{
	}
	
	FItemTypeSettingsInfo(const FName& Name)
		: ItemTypeDisplayName(FText::FromName(Name))
		, DropClass(nullptr)
	{
	}
	
	UPROPERTY(EditAnywhere)
	FText ItemTypeDisplayName;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ADDropReward> DropClass;
};

USTRUCT()
struct FItemTypeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FItemTypeSettingsInfo Weapon;

	UPROPERTY(EditAnywhere)
	FItemTypeSettingsInfo Module;
	
	UPROPERTY(EditAnywhere)
	FItemTypeSettingsInfo Material;
	
	UPROPERTY(EditAnywhere)
	FItemTypeSettingsInfo Consumable;
	
	UPROPERTY(EditAnywhere)
	FItemTypeSettingsInfo Experience;
	
	UPROPERTY(EditAnywhere)
	FItemTypeSettingsInfo Ability;
};

USTRUCT()
struct FFireModeDisplayNames
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText SemiAutomatic;

	UPROPERTY(EditAnywhere)
	FText FullyAutomatic;
	
	UPROPERTY(EditAnywhere)
	FText Accumulation;
};

USTRUCT()
struct FWeaponTypeDisplayNames
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText AssaultRifle;

	UPROPERTY(EditAnywhere)
	FText GrenadeLaunch;
	
	UPROPERTY(EditAnywhere)
	FText Shotgun;
	
	UPROPERTY(EditAnywhere)
	FText SniperRifle;
	
	UPROPERTY(EditAnywhere)
	FText PrecisionRifle;
};

USTRUCT()
struct FItemQualities
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FQualityInfo Normal;

	UPROPERTY(EditAnywhere)
	FQualityInfo Advanced;

	UPROPERTY(EditAnywhere)
    FQualityInfo Rare;
    
	UPROPERTY(EditAnywhere)
    FQualityInfo Epic;

	UPROPERTY(EditAnywhere)
    FQualityInfo Legendary;
};


USTRUCT()
struct FMagazineDropClasses
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<ADDropMagazine> AmmoL1;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ADDropMagazine> AmmoL2;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ADDropMagazine> AmmoL3;
};


UCLASS(Blueprintable)
class ULevelListAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FLevelInformation> Levels;
};

/**
 * 
 */
UCLASS(Config=Engine, defaultconfig)
class DREAM_API UDProjectSettings : public UObject
{
	GENERATED_BODY()

	UDProjectSettings();
	
public:

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	FText GetWeaponTypeName(EWeaponType WeaponType) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	FText GetWeaponFireMode(EFireMode FireMode) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
    FText GetItemTypeName(TEnumAsByte<EItemType::Type> ItemType) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const FQualityInfo& GetQualityInfo(EPropsQuality Quality) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const FSoftObjectPath& GetMainUILevel() const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	FString GetMainUILevelAsString() const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const struct FSurfaceImpactEffect& GetSurfaceImpactEffect(EPhysicalSurface SurfaceType) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	ULevelListAsset* GetLevelListAsset() const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const FName& GetWepActiveSockName() const;
	
	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const FName& GetWepHolsterSockName() const;

public:
	
	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	static UDProjectSettings* GetProjectSettings();

public:

	UClass* GetMasterAnimClass() const;
	
	UClass* GetSlaveAnimClass() const;
	
	const FItemDef& GetItemDefinition(int32 ItemGuid) const;

	UClass* GetItemClassFromGuid(int32 ItemGuid) const;

	TArray<FTalentInfo> GetTalents(ETalentCategory Category, int64 Talents) const;
	
	TArray<FTalentInfo> GetLearnedTalents(ETalentCategory Category, int64 LearnedTalents) const;

	class USurfaceImpactAsset* GetSurfaceImpactAsset() const;

	UClass* GetDamageComponentClass() const;

	float GetPickupAmmunitionAmount() const;

	float GetIdleSwitchingTime() const;

	const FItemTypeSettingsInfo& GetItemTypeInfo(EItemType::Type ItemType) const;

	TSubclassOf<ADDropReward> GetRewardDropClass(EItemType::Type ItemType);

	TSubclassOf<ADDropMagazine> GetMagazineDropClass(EAmmoType AmmoType);

	UDataTable* GetItemTable() const;
	
protected:

	UPROPERTY(Config, EditAnywhere, Category=ProjectSettings)
	FWeaponTypeDisplayNames WeaponTypeDisplayNames;


	/**
	* 物品质量配置项
	*/
	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	FItemQualities ItemQualities;
	
	/**
	* 	关卡数据
	*/
	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="LevelListAsset"), Category = ProjectSettings)
	FSoftObjectPath LevelAsset;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="SurfaceImpactAsset"), Category = ProjectSettings)
	FSoftObjectPath SurfaceImpactAsset;

	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	FItemTypeSettings ItemTypeSettings;

	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	FFireModeDisplayNames FireModeDisplayNames;

	
	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	FMagazineDropClasses MagazineDropSettings;

	/**
	* 用作主界面的世界
	*/
	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "World"), Category = ProjectSettings)
	FSoftObjectPath MainInterfaceWorld;

	/**
	 * 玩家控制的角色的动画
	 */
	UPROPERTY(Config, EditAnywhere, Meta = (MetaClass = "AnimInstance"), Category = ProjectSettings)
	FSoftClassPath PawnMasterAnimClass;

	/**
	* 玩家控制的角色的附加网格体的动画
	*/
	UPROPERTY(Config, EditAnywhere, Meta = (MetaClass = "AnimInstance"), Category = ProjectSettings)
	FSoftClassPath PawnSlaveAnimClass;

	
	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "DataTable"), Category = ProjectSettings)
	FSoftObjectPath ItemsTable;

	
	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "TalentAsset"), Category = ProjectSettings)
	FSoftObjectPath TalentAsset;

	/** 武器激活时的插槽 */
	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	FName WeaponActivateSocketName;

	/**
	 * 武器收起时的插槽
	 */
	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	FName WeaponHolsterSocketName;

	/**
	 * 战斗状态自动切换到空闲时需要的时间
	 */
	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	float CombatToIdleTime;

	UPROPERTY(Config, EditAnywhere, Meta = (MetaClass = "DamageWidgetComponent"), Category = ProjectSettings)
	FSoftClassPath DamageWidgetClass;

	/** 拾取弹药时应该随机恢复的范围 */
	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	FRangeRandomFloat PickupAmmunitionAmount;
};
