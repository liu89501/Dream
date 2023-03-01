// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PDIFunctions.generated.h"

class UDRewardPool;

DECLARE_DYNAMIC_DELEGATE_OneParam(FPDIFOnPropertiesChange, const FPlayerProperties&, Properties);

/**
 * 
 */
UCLASS()
class DREAM_API UPDIFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = "DreamStatics|PDI")
    static TSubclassOf<class AShootWeapon> SoftClassPathToWeaponClass(const FSoftClassPath& SoftClassPath);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = "DreamStatics|PDI")
    static TSubclassOf<class UDModuleBase> SoftClassPathToModuleClass(const FSoftClassPath& SoftClassPath);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "ToQuestDesc"), Category = "DreamStatics|PDI")
    static class UDQuestDescription* SoftObjectToDQuestDescription(const FSoftObjectPath& ObjectPath);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "ItemArray"), Category = "DreamStatics|PDI")
	static void MakeItemArray(const FItemListHandle& ItemListHandle, TArray<FItemHandle>& ItemArray);
	
	UFUNCTION(BlueprintPure, Category = "DreamStatics|PDI")
	static const FPlayerProperties& GetCachedPlayerProperties();

	UFUNCTION(BlueprintPure, Category = "DreamStatics|PDI")
	static int32 GetPlayerMaterialNum(int32 ItemGuid);

	/*UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI")
	static void ConsumeMaterial(const TArray<FAcquisitionCost>& Costs);
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI")
    static void ConsumeMaterialFromHandle(const FCostsHandle& Handle);*/
	
	UFUNCTION(BlueprintPure, Category = "DreamStatics|PDI")
	static bool CheckMaterials(const TArray<FAcquisitionCost>& Costs);
	
	UFUNCTION(BlueprintPure, Category = "DreamStatics|PDI")
	static bool CheckMaterialsFromHandle(const FCostsHandle& Handle);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "DreamStatics|PDI")
	static void SendUpdateTaskCondForEvent(APlayerController* PlayerCtrl, FName EventName);

	/** 给关卡中的所有玩家发送任务事件 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Meta = (WorldContext="WorldContextObject"), Category = "DreamStatics|PDI")
	static void SendUpdateTaskCondForEventToAll(UObject* WorldContextObject, FName EventName);

	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf="Object", HidePin = "Object"), Category = "DreamStatics|PDI|Delegate")
	static void BindPropertiesChangeDelegate(UPARAM(DisplayName="Event") FPDIFOnPropertiesChange Delegate, FMulticastDelegateHandle& Handle);
	
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI|Delegate")
	static void RemovePropertiesDelegateHandle(const FMulticastDelegateHandle& MulticastDelegateHandle);
	
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI")
	static void GroupModules(const TArray<FPlayerModule>& Modules, TMap<EModuleCategory, FPlayerModuleList>& GroupModules);

	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI")
	static void GetEquippedModule(const TArray<FPlayerModule>& Modules, TArray<FPlayerModule>& EquippedModules);

	UFUNCTION(BlueprintPure, Category="DreamStatics|PDI")
    static bool IsConditionCompleted(const FTaskInformation& Task);

	UFUNCTION(BlueprintPure, Category="DreamStatics|PDI")
	static float GetConditionPercentage(const FQuestConditionHandle& Handle);

	UFUNCTION(BlueprintCallable, Category="DreamStatics|PDI")
	static FItemHandle MakeSimpleItemHandle(FItemGuidHandle Guid, int32 Num);


	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Meta = (WorldContext="WorldContextObject"), Category = "DreamStatics|PDI")
    static void SpawnRewardsAtLocation(APlayerController* PlayerController, UDRewardPool* RewardPool, const FVector& Location);
	
	/** 给当前所有玩家生成奖励 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Meta = (WorldContext="WorldContextObject"), Category = "DreamStatics|PDI")
    static void GenerateRewardsForAllPlayers(UObject* WorldContextObject, UDRewardPool* RewardPool, const FVector& Location);
};
