// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "ShootWeapon.h"
#include "UObject/NoExportTypes.h"
#include "DProjectSettings.generated.h"

struct FTalentInfo;

struct FItemDetails
{
	int32 ItemGuid;
	FString ItemClass;
};

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
	const FSurfaceImpactEffect& GetSurfaceImpactEffect(EPhysicalSurface SurfaceType) const;

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
	
	const FItemDefinition& GetItemDefinition(int32 ItemGuid) const;

	UClass* GetItemClassFromGuid(int32 ItemGuid) const;

	bool GetAllItems(EItemType::Type ItemType, TArray<FItemDetails>& Items) const;

	TArray<FTalentInfo> GetTalents(ETalentCategory Category, int64 Talents) const;
	
	TArray<FTalentInfo> GetLearnedTalents(ETalentCategory Category, int64 LearnedTalents) const;

	class USurfaceImpactAsset* GetSurfaceImpactAsset() const;

	UClass* GetDamageComponentClass() const;
	
protected:

	UPROPERTY(Config, EditAnywhere, Category=ProjectSettings)
	TMap<EWeaponType, FText> WeaponTypeName;

	/**
	* 物品品级配置项
	*/
	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	TMap<EPropsQuality, FQualityInfo> QualitySettings;
	
	/**
	* 	关卡数据
	*/
	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="LevelListAsset"), Category = ProjectSettings)
	FSoftObjectPath LevelAsset;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="SurfaceImpactAsset"), Category = ProjectSettings)
	FSoftObjectPath SurfaceImpactAsset;

	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	TMap<TEnumAsByte<EItemType::Type>, FText> ItemNameSettings;

	UPROPERTY(Config, EditAnywhere, Category = ProjectSettings)
	TMap<EFireMode, FText> FireModeNameSettings;

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

	UPROPERTY(Config, EditAnywhere, Meta = (MetaClass = "DamageWidgetComponent"), Category = ProjectSettings)
	FSoftClassPath DamageWidgetClass;

};
