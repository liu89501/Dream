// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DCharacterBase.h"
#include "GameFramework/Character.h"
#include "DCharacterPlayerBase.generated.h"

class AShootWeapon;
class ADPlayerController;
class UDModuleBase;
struct FEquipmentAttributes;
struct FPlayerInfo;
struct FPlayerProperties;

UCLASS(Blueprintable)
class UCharacterMesh : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* MasterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<USkeletalMesh*> SlaveMeshs;
};

UCLASS()
class DREAM_API ADCharacterPlayerBase : public ADCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADCharacterPlayerBase();

	/**
	 * 默认属性值配置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|Attribute")
	class UDBaseAttributesAsset* BaseAttributesSettings;

public:

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	AShootWeapon* GetActiveWeapon() const;

	/**
	 * 获取当前玩家正在使用的是几号位的武器
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	int32 GetActiveWeaponIndex() const;

	UFUNCTION(BlueprintCallable, Category = "CharacterPlayer")
	class ADPlayerController* GetPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void EquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void EquipModule(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs);

	UFUNCTION(BlueprintCallable, Category = CharacterPlayer)
	void LearningTalents(const TArray<TSubclassOf<class UDreamGameplayAbility>>& TalentClasses);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=CharacterPlayer)
	void SwitchWeapon(int32 NewWeaponIndex);

	UFUNCTION(BlueprintCallable, Category=CharacterPlayer)
	void SetCharacterMesh(UCharacterMesh* CharacterMesh);

public:

	/** Camera 相关 */
	void SetCameraFieldOfView(float NewFOV);
	void CameraAimTransformLerp(float Alpha);
	
	/** GameplayAbility 相关 */

	/**
	 *	通过GameplayTag 触发Ability
	 *	@param Target Ability应用的目标 如果有的话 没有就是自身
	 */
	void TriggerAbilityFromTag(const FGameplayTag& Tag, AActor* Target);

	FORCEINLINE UCharacterMesh* GetCharacterMesh() const
	{
		return CurrentCharacterMesh;
	}

protected:

	UFUNCTION()
	void OnRep_ActiveWeapon();

	/** 装备武器(切换武器) */
	void SwitchWeapon();
	void HandleSwitchWeapon(int32 WeaponIndex);
	void SwitchFinished(int32 WeaponIndex);
	UFUNCTION(Reliable, Server)
	void ServerSwitchWeapon();
	UFUNCTION(Reliable, NetMulticast)
	void MulticastSwitchWeapon();

	void SwitchWeaponToFirst();
	void SwitchWeaponToSecond();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, TSubclassOf<AShootWeapon> NewWeaponClass);
	void DoServerEquipWeapon(int32 Index, const FEquipmentAttributes& Attrs, UClass* NewWeaponClass);
	
	UFUNCTION(Server, Reliable)
	void ServerEquipModule(TSubclassOf<UDModuleBase> ModuleClass, const FEquipmentAttributes& Attrs);
	void DoServerEquipModule(UClass* ModuleClass, const FEquipmentAttributes& Attrs);

	UFUNCTION(Server, Reliable)
	void ServerLearningTalent(const TArray<TSubclassOf<UDreamGameplayAbility>>& TalentClasses);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetActiveWeapon(AShootWeapon* NewWeapon);

	virtual void Destroyed() override;

	void OnInitPlayer(const FPlayerInfo& PlayerInfo, bool bSuccess);

	void OnPlayerPropertiesChanged(const FPlayerProperties& Properties);

	/** 属性相关 */
	void AdditiveAttributes(const FEquipmentAttributes& Attributes);
    void RefreshAttributeBaseValue();
	
	UFUNCTION(Server, Reliable)
    void ServerInitializePlayer(const FPlayerInfo& PlayerInfo);

	UFUNCTION(Server, Reliable)
	void ServerUpdateCharacterMesh(UCharacterMesh* CharacterMesh);

	UFUNCTION()
	void OnRep_CharacterMesh();

	void UpdateCharacterMesh();

protected:

	UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon)
	AShootWeapon* ActiveWeapon;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterMesh, Category = CharacterPlayer)
	UCharacterMesh* CurrentCharacterMesh;

	/** 当前正在使用的武器 在 WeaponInventory 的索引 */
	UPROPERTY(Replicated)
	int32 ActiveWeaponIndex;

	/** 当前玩家已装备的武器 仅服务器有效 */
	UPROPERTY()
	TArray<AShootWeapon*> WeaponInventory;
	/** 当前装备的模块 */
	UPROPERTY()
	TArray<UDModuleBase*> EquippedModules;

	UPROPERTY()
	TArray<TSubclassOf<UDreamGameplayAbility>> LearnedTalents; 

	static class UDProjectSettings* CDOProjectSettings;
	
private:

	FDelegateHandle Handle_Properties;
	FDelegateHandle Handle_PlayerInfo;

	
};

