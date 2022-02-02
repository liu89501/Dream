// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DDropMagazine.h"
#include "DPropsType.h"
#include "DreamType.h"
#include "UObject/NoExportTypes.h"
#include "DMProjectSettings.generated.h"

#define DEFAULT_MAX_PLAYERS 4

class ADDropReward;
class UTalentAsset;
class UDBaseAttributesAsset;
class UDMProjectSettingsInstance;
class USurfaceImpactAsset;
class UCurveVector;
class UDMMantleAsset;

struct FTalentInfo;

enum class EWeaponType : uint8;
enum class EFireMode : uint8;
enum class ETalentCategory : uint8;
enum class EAmmoType : uint8;
enum class EMantleType : uint8;
enum class EOverlayState : uint8;


USTRUCT(BlueprintType)
struct FLevelInformation
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "World"), BlueprintReadOnly)
	FSoftObjectPath Map;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString GameModeClassAlias;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* MapIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;
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

USTRUCT()
struct FMantleTraceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float MaxLedgeHeight;
	
	UPROPERTY(EditAnywhere)
	float MinLedgeHeight;

	UPROPERTY(EditAnywhere)
	float ReachDistance;

	UPROPERTY(EditAnywhere)
	float ForwardTraceRadius;

	UPROPERTY(EditAnywhere)
	float DownwardTraceRadius;
};

UCLASS(Blueprintable)
class ULevelListAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FLevelInformation> Levels;
};

extern DREAM_API UDMProjectSettingsInstance* GSProject;

/**
 * 
 */
UCLASS(Config=Engine, defaultconfig)
class DREAM_API UDMProjectSettings : public UObject
{
	GENERATED_BODY()

	UDMProjectSettings();
	
public:

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	FText GetWeaponTypeName(EWeaponType WeaponType) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	FText GetWeaponFireMode(EFireMode FireMode) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
    FText GetItemTypeName(TEnumAsByte<EItemType::Type> ItemType) const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const FSoftObjectPath& GetMainUILevel() const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	FString GetMainUILevelAsString() const;

	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const FName& GetWepActiveSockName() const;
	
	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	const FName& GetWepHolsterSockName() const;

public:
	
	UFUNCTION(BlueprintPure, Category=DreamProjectSettings)
	static UDMProjectSettings* GetProjectSettings();

	static const UDMProjectSettings* Get();

public:

	const FQualityInfo& GetQualityInfo(EPropsQuality Quality) const;

	float GetPickupAmmunitionAmount() const;

	float GetIdleSwitchingTime() const;

	const FItemTypeSettingsInfo& GetItemTypeInfo(EItemType::Type ItemType) const;

	TSubclassOf<ADDropReward> GetRewardDropClass(EItemType::Type ItemType) const;

	TSubclassOf<ADDropMagazine> GetMagazineDropClass(EAmmoType AmmoType) const;

	const FMantleTraceSettings& GetMantleTraceSettings() const;

protected:

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

private:

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
	UPROPERTY(Config, EditAnywhere, Meta = (MetaClass = "AnimInstance"), Category="ProjectSettings|Animation")
	FSoftClassPath PawnMasterAnimClass;

	/**
	* 玩家控制的角色的附加网格体的动画
	*/
	UPROPERTY(Config, EditAnywhere, Meta = (MetaClass = "AnimInstance"), Category="ProjectSettings|Animation")
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

	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "DBaseAttributesAsset"), Category = "ProjectSettings|Character")
	FSoftObjectPath PlayerBaseAttributes;

	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "CurveVector"), Category="ProjectSettings|Character")
	FSoftObjectPath MovementCurve;
	
	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "CurveFloat"), Category="ProjectSettings|Character")
	FSoftObjectPath RotationRateCurve;

	UPROPERTY(Config, EditAnywhere, Category="ProjectSettings|Mantle")
	FMantleTraceSettings MantleTraceSettings;

	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "CurveFloat"), Category="ProjectSettings|Mantle")
	FSoftObjectPath MantlePositionCurve;
	
	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "CurveFloat"), Category="ProjectSettings|Mantle")
	FSoftObjectPath MantlingRotationCurve;
	
	UPROPERTY(Config, EditAnywhere, Meta = (AllowedClasses = "DMMantleAsset"), Category="ProjectSettings|Character")
	FSoftObjectPath MantleAsset;

private:

	friend class UDMProjectSettingsInstance;
};

UCLASS()
class DREAM_API UDMProjectSettingsInstance : public UObject
{
	
	GENERATED_BODY()

public:

	void Initialize();

	virtual void FinishDestroy() override;

	UClass* GetMasterAnimClass() const;
	UClass* GetSlaveAnimClass() const;
	UClass* GetDamageWidgetClass() const;
	ULevelListAsset* GetLevelListAsset() const;
	USurfaceImpactAsset* GetSurfaceImpactAsset() const;
	UTalentAsset* GetTalentAsset() const;
	UDBaseAttributesAsset* GetPlayerBaseAttributes() const;
	UClass* GetItemClassFromGuid(int32 ItemGuid) const;
    const struct FSurfaceImpactEffect& GetSurfaceImpactEffect(EPhysicalSurface SurfaceType) const;
	const FItemDef& GetItemDefinition(int32 ItemGuid) const;
	void GetTalents(ETalentCategory Category, int64 Talents, TArray<FTalentInfo>& AllTalents) const;
	void GetLearnedTalents(ETalentCategory Category, int64 Talents, TArray<FTalentInfo>& LearnedTalents) const;
	UDataTable* GetItemTable() const;
	UClass* GetDamageComponentClass() const;
	FBaseAttributes GetBaseAttributes(int32 Level) const;
	UCurveFloat* GetRotationRateCurve() const;
	UCurveVector* GetMovementCurve() const;
	UCurveFloat* GetMantlePositionCurve() const;
	UCurveFloat* GetMantlingRotationCurve() const;
	UDMMantleAsset* GetMantleAsset() const;
	
	const struct FMantleInformation& GetMantleInfo(EMantleType MantleType, EOverlayState OverlayState) const;

private:

	void InitializeSettingsInstances();

private:

	UPROPERTY()
	UClass* PawnMasterAnimClass;
	
	UPROPERTY()
	UClass* PawnSlaveAnimClass;

	UPROPERTY()
	UClass* DamageWidgetClass;

	UPROPERTY()
	ULevelListAsset* LevelListAsset;

	UPROPERTY()
	USurfaceImpactAsset* SurfaceImpactAsset;

	UPROPERTY()
	UDataTable* ItemListTable;

	UPROPERTY()
	UTalentAsset* TalentAsset;

	UPROPERTY()
	UDBaseAttributesAsset* PlayerBaseAttributes;

	UPROPERTY()
	UCurveFloat* RotationRateCurve;
	
	UPROPERTY()
	UCurveVector* MovementCurve;
	
	UPROPERTY()
	UCurveFloat* MantlePositionCurve;
	
	UPROPERTY()
	UCurveFloat* MantlingRotationCurve;
	
	UPROPERTY()
	UDMMantleAsset* MantleAsset;


	friend class UDMProjectSettings;
	
};
