// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DModuleBase.h"
#include "DreamType.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilityTypes.h"
#include "HoldState.h"
#include "PlayerDataInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DGameplayStatics.generated.h"

class APlayerController;
class ACharacter;

USTRUCT(BlueprintType)
struct FImpactMontages
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UAnimMontage* FHitMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* BHitMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* LHitMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* RHitMontage;
};

/**
 * 
 */
UCLASS()
class DREAM_API UDGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static UClass* LoadUClass(FString QualifiedName);

	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static FString GetClassPathName(TSubclassOf<UObject> ObjectClass);

	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static const FLinearColor& GetGameThemeColor();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
	static bool GetQualityInfo(UObject* WorldContextObject, EPropsQuality Quality, FQualityInfo& QualityInfo);

	UFUNCTION(BlueprintCallable, Category = DreamStatics)
	static bool ContainsActionKey(APlayerController* PlayerController, FKey Key, FName ActionName);

	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static FName GetInputActionKeyName(APlayerController* PlayerController, FName ActionName);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
	static void ServerTravel(UObject* WorldContextObject, const FString& URL, bool bAbsolute);

	/* DATE 转换为 文本 */
	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static FText ToTimeText(int32 TotalSeconds);

	/**
	* 获取默认的重力值
	*/
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
	static float GetDefaultGravityZ(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category = DreamStatics)
	static void AddIconToPlayerMinimap(UObject* WorldContextObject, AActor* Actor);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category = DreamStatics)
	static void RemoveIconInPlayerMinimap(UObject* WorldContextObject, AActor* Actor);

	/**
	* 	替换widget中的子项
	*/
	static void ReplaceWidgetChildAt(class UPanelWidget* ParentWidget, int32 ChildIndex, class UWidget* NewWidget);

	/* 停止匹配 */
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Matchmaking")
	static void StopMatchmaking(const FMatchmakingHandle& Handle);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Matchmaking")
	static int32 GetSessionPlayers();

	UFUNCTION(BlueprintPure, Category = DreamStatics)
    static USceneComponent* GetAttachComponentFromSocketName(USceneComponent* ParentComponent, const FName& SocketName);

	/**
	 * 线性检测后并且触发GameplayEvent
	 */
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities")
	static bool LineTraceAndSendEvent(AActor* Source, FGameplayTag InEventTag, FVector TraceStart,
		FVector TraceEnd, ECollisionChannel Channel, FHitResult& OutHit);

	/**
	 * 球体检测后并且触发GameplayEvent
	 */
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities")
	static bool SphereTraceAndSendEvent(
		AActor* Source,
		FGameplayTag InEventTag,
		FVector Origin,
		float Radius,
		ETraceTypeQuery TraceChannel,
		bool bTraceComplex,
		TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType);

	/**
	 *  创建伤害widget
	 */
	UE_DEPRECATED(4.26, "暂时弃用")
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities")
	static void SpawnDamageWidgets(AActor* TargetActor, const struct FGameplayCueParameters& Parameters,
	                               bool bHealthSteal);

	static struct FDreamGameplayEffectContext* MakeDreamEffectContext(AActor* SourceActor, float DamageFalloff,
	                                                                  const FHitResult& Hit, const FVector& Origin);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Abilities")
	static FGameplayEffectContextHandle MakeDreamEffectContextHandle(AActor* SourceActor, float DamageFalloff,
	                                                                 const FHitResult& Hit, const FVector& Origin);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Abilities")
	static class UDGameplayEffectUIData* GetGameplayUIData(const TSubclassOf<UDreamGameplayAbility>& AbilityClass);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "SoftClassToAbilityClass"), Category = "DreamStatics|Abilities")
	static TSubclassOf<UDreamGameplayAbility> SoftClassToDAbility(const FSoftClassPath& SoftClassPath);

	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities", meta = (HidePin="Ability", DefaultToSelf = "Ability"))
	static bool ApplyGameplayEffectToAllActors(class UGameplayAbility* Ability, const FGameplayEventData& EventData, TSubclassOf<class UGameplayEffect> EffectClass);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "DreamStatics|Abilities")
	static void ApplyModToAttribute(AActor* Source, FGameplayAttribute Attribute, TEnumAsByte<EGameplayModOp::Type> ModifierOp, float ModifierMagnitude);

	/**
	 *  获取Actor玩家控制器, 如果没找到会返回null
	 */
	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static APlayerController* GetActorPlayerController(AActor* Actor);

	/** 返回到主菜单界面 */
	UFUNCTION(BlueprintCallable, Category = DreamStatics)
	static void ReturnToMainMenuWithTextReason(APlayerController* PlayerCtrl, FText Reason);


	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static void CalculateFBDirection(const FVector& Velocity, const FRotator& BaseRotation, float& Angle, bool& bIsBackward);

	static void SpawnWeaponTrailParticles(UObject* WorldContextObject, const FWeaponTrailVFX& TrailVfx,
	                                      const FVector& StartLocation, const FVector& EndLocation);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Widget")
	static int32 GetWidgetZOrder(TEnumAsByte<EWidgetOrder::Type> Type);

	/** 聚焦到viewport */
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf="Widget", HidePin="Widget"), Category = "DreamStatics|Widget")
	static void FocusOnViewport(UWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = DreamStatics)
	static void ForceDestroyComponent(UActorComponent* Component);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = "DreamStatics|PDI")
	static UItemDataWeapon* WeaponCastToExInformation(const FPlayerWeapon& Weapon);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = "DreamStatics|PDI")
	static UItemDataModule* ModuleCastToExInformation(const FPlayerModule& Module);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|PDI")
	static const FPlayerProperties& GetCachedPlayerProperties();

	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf="Object", HidePin = "Object"), Category = "DreamStatics|PDI|Delegate")
	static void BindMoneyChangedDelegate(UObject* Object, FName FunctionName, FMulticastDelegateHandle& Handle);
	
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf="Object", HidePin = "Object"), Category = "DreamStatics|PDI|Delegate")
	static void BindExperienceChangedDelegate(UObject* Object, FName FunctionName, FMulticastDelegateHandle& Handle);
	
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI|Delegate")
	static void RemoveExperienceDelegateHandle(const FMulticastDelegateHandle& MulticastDelegateHandle);
	
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI|Delegate")
	static void RemoveMoneyDelegateHandle(const FMulticastDelegateHandle& MulticastDelegateHandle);
	
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI")
	static void GroupModules(const TArray<FPlayerModule>& Modules, TMap<EModuleCategory, FPlayerModuleList>& GroupModules);

	UFUNCTION(BlueprintCallable, Category = "DreamStatics|PDI")
	static void GetEquippedModule(const TArray<FPlayerModule>& Modules, TArray<FPlayerModule>& EquippedModules);

	UFUNCTION(BlueprintPure, Meta=(CompactNodeTitle = "GetAllItems"), Category="DreamStatics|PDI")
    static void GetAllItems(UItemData* ItemData, TArray<UItemData*>& Items);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = DreamStatics)
	static TSubclassOf<class AShootWeapon> SoftClassPathToWeaponClass(const FSoftClassPath& SoftClassPath);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = DreamStatics)
	static TSubclassOf<class UDModuleBase> SoftClassPathToModuleClass(const FSoftClassPath& SoftClassPath);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = DreamStatics)
	static UObject* TryLoadObject(const FSoftObjectPath& ObjectPath);
	
	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "SoftObjectToDescription"), Category = DreamStatics)
	static class UDQuestDescription* SoftObjectToDQuestDescription(const FSoftObjectPath& ObjectPath);

	UFUNCTION(BlueprintCallable, Category="DreamStatics|DreamTask")
	static void ClearHoldStateHandle(UObject* WorldContextObject, const FHoldStateHandle& Handle);


	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf="Character", HidePin="Character"), Category="DreamStatics")
	static void PlayImpactAnim(ACharacter* Character, const FImpactMontages& HitMontages, float RotationYaw);

	/** Sets a Niagara StaticMesh parameter by name, overriding locally if necessary.*/
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Niagara", meta = (DisplayName = "Set Niagara Spline Component"))
    static void OverrideSystemUserVariableSplineComponent(class UNiagaraComponent* NiagaraSystem, const FString& OverrideName, class USplineComponent* SplineComponent);

	UFUNCTION(BlueprintCallable, Category="DreamStatics|Misc")
	static void CloseGame();

	UFUNCTION(BlueprintPure, Meta = (WorldContext="WorldContextObject"), Category = "DreamStatics|Settings")
    static ULevelListAsset* GetLevelList(UObject* WorldContextObject);
};
