// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DreamGameplayType.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilityTypes.h"
#include "HoldState.h"
#include "MatchmakingCallProxy.h"
#include "PlayerDataInterface.h"
#include "PlayerGameData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DGameplayStatics.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnEmptyArgsDelegate);

class APlayerController;
class ACharacter;

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

	UFUNCTION(BlueprintCallable, Category = DreamStatics)
	static bool ContainsActionKey(APlayerController* PlayerController, const FKey& Key, FName ActionName);

	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static FName GetInputActionKeyName(APlayerController* PlayerController, FName ActionName);


	/**
	* 获取默认的重力值
	*/
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
	static float GetDefaultGravityZ(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category = "DreamStatics|Minimap")
	static void AddIconToPlayerMinimap(UObject* WorldContextObject, AActor* Actor);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category = "DreamStatics|Minimap")
	static void RemoveIconInPlayerMinimap(UObject* WorldContextObject, AActor* Actor);

	/* 停止匹配 */
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category = "DreamStatics|Matchmaking")
	static void StopMatchmaking(UPARAM(ref) FMatchmakingHandle& Handle);
	

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
	 * 球体检测后并且应用效果
	 */
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities")
	static bool SphereTraceAndApplyEffect(
		AActor* Source,
		FVector Origin,
		float Radius,
		ECollisionChannel TraceChannel,
		TSubclassOf<UGameplayEffect> ApplyEffect,
		bool bIgnoredSelf = true,
		bool bTraceComplex = false,
		bool bForceIncludeHitResult = false,
		EDDamageType EffectDamageType = EDDamageType::Other);
	

	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities", meta = (HidePin="Ability", DefaultToSelf = "Ability"))
    static bool ApplyGameplayEffectToAllActors(
        class UGameplayAbility* Ability,
        const FGameplayEventData& EventData,
        TSubclassOf<class UGameplayEffect> EffectClass,
        bool bForceIncludeHitResult,
        EDDamageType EffectDamageType);


	/**
	 *  创建伤害widget
	 */
	UE_DEPRECATED(4.26, "暂时弃用")
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities")
	static void SpawnDamageWidgets(AActor* TargetActor, const struct FGameplayCueParameters& Parameters, bool bHealthSteal);

	static struct FDreamGameplayEffectContext* MakeDreamEffectContext(AActor* SourceActor, float DamageFalloff, const FHitResult& Hit);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Abilities")
	static FGameplayEffectContextHandle MakeDreamEffectContextHandle(AActor* SourceActor, float DamageFalloff, const FHitResult& Hit);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Abilities")
	static class UDGameplayEffectUIData* GetGameplayUIData(UClass* AbilityClass);

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "SoftClassToAbilityClass"), Category = "DreamStatics|Abilities")
	static TSubclassOf<UDreamGameplayAbility> SoftClassToDAbility(const FSoftClassPath& SoftClassPath);


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

	/**
	 * Orientation: 0 - Forward, 1 - Right, 2 - Backward, 3 - Left
	 */
	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static void CalculateOrientation(const FVector& Velocity, const FRotator& BaseRotation, float& Angle, int32& Orientation);

	/** Copy From UAnimInstance */
	static float CalculateDir(const FVector& Velocity, const FRotator& BaseRotation);

	
	static void SpawnWeaponTrailParticles(UObject* WorldContextObject, const struct FWeaponTrailVFX& TrailVfx,
	                                      const FVector& StartLocation, const FVector& EndLocation);
	

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = DreamStatics)
	static UObject* TryLoadObject(const FSoftObjectPath& ObjectPath);

	UFUNCTION(BlueprintCallable, Category="DreamStatics|DreamTask")
	static void ClearHoldStateHandle(UObject* WorldContextObject, const FHoldStateHandle& Handle);

	/** Sets a Niagara StaticMesh parameter by name, overriding locally if necessary.*/
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Niagara", meta = (DisplayName = "Set Niagara Spline Component"))
    static void OverrideSystemUserVariableSplineComponent(class UNiagaraComponent* NiagaraSystem, const FString& OverrideName, class USplineComponent* SplineComponent);

	UFUNCTION(BlueprintCallable, Category="DreamStatics|Misc")
	static void CloseGame();

	UFUNCTION(BlueprintCallable, Meta = (WorldContext="WorldContextObject"), Category = "DreamStatics|Misc")
    static void ReturnToHomeWorld(UObject* WorldContextObject);

	/* DATE 转换为 文本 */
	UFUNCTION(BlueprintPure, Category = "DreamStatics|Misc")
    static FText ToTimeText(int32 TotalSeconds);
    
	UFUNCTION(BlueprintPure, Category = "DreamStatics|Misc")
    static FString GetLocalPlayerName();

	/* 将Target的所有子组件复制到Source中 */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext="WorldContextObject"), Category = "DreamStatics|Misc")
    static void CopyMeshComponents(UObject* WorldContextObject, UMeshComponent* Source, UMeshComponent* Target);
	
	UFUNCTION(BlueprintPure, Category = "DreamStatics|Settings")
    static FName GetWeaponSocketName(bool bMasterSocket);
	
	UFUNCTION(BlueprintPure, Category = "DreamStatics|Settings")
	static const FItemDef& GetItemDef(int32 ItemGuid);
	
	UFUNCTION(BlueprintPure, Category = "DreamStatics|Settings")
	static ULevelListAsset* GetLevelListAsset();
	
};
