// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "DreamType.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DGameplayStatics.generated.h"

class APlayerController;

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
	static const FPropsInfo& GetPropsInfoFromClassName(const FString& ClassString);
	/** 通过UClass获取道具信息 */
	UFUNCTION(BlueprintPure, Category = DreamStatics)
    static const FPropsInfo& GetPropsInfoFromClass(UClass* PropsClass);
	
	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static const FEquipmentAttributes& GetEquipmentAttributesFromClassName(const FString& ClassString);
	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static const FEquipmentAttributes& GetEquipmentAttributesFromClass(UClass* PropsClass);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
	static FLinearColor GetGameThemeColor(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
	static bool GetQualityInfo(UObject* WorldContextObject, EPropsQuality Quality, FQualityInfo& QualityInfo);

	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static FLinearColor ParseColorFromString(const FString& ColorString);

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
    static void RemoveIconToPlayerMinimap(UObject* WorldContextObject, AActor* Actor);

	/**
	* 	替换widget中的子项
	*/
	static void ReplaceWidgetChildAt(class UPanelWidget* ParentWidget, int32 ChildIndex, class UWidget* NewWidget);

	/* 停止匹配 */
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Matchmaking")
	static void StopMatchmaking(const FMatchmakingHandle& Handle);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Matchmaking")
	static int32 GetJoinedPlayerNum();

	/**
	 * 线性检测后并且触发GameplayEvent
	 */
	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities")
	static bool LineTraceAndSendEvent(
		AActor* Source,
		FGameplayTag InEventTag,
		FVector TraceStart,
		FVector TraceEnd,
		ECollisionChannel Channel,
		FHitResult& OutHit);

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
	static void SpawnDamageWidgets(AActor* TargetActor, const struct FGameplayCueParameters& Parameters, bool bHealthSteal);

	
	static struct FDreamGameplayEffectContext* MakeDreamEffectContext(AActor* SourceActor, float DamageFalloff, const FHitResult& Hit, const FVector& Origin);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Abilities")
	static FGameplayEffectContextHandle MakeDreamEffectContextHandle(AActor* SourceActor, float DamageFalloff, const FHitResult& Hit, const FVector& Origin);

	UFUNCTION(BlueprintPure, Category = "DreamStatics|Abilities")
    static class UDGameplayEffectUIData* GetGameplayUIData(TSubclassOf<class UDreamGameplayAbility> AbilityClass);

	UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities", meta = (HidePin="Ability", DefaultToSelf = "Ability"))
    static bool ApplyGameplayEffectToAllActors(class UGameplayAbility* Ability, const FGameplayEventData& EventData, TSubclassOf<class UGameplayEffect> EffectClass);

	/**
	 *  获取Actor玩家控制器, 如果没找到会返回null
	 */
	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static APlayerController* GetActorPlayerController(AActor* Actor);

	/** 返回到主菜单界面 */
	UFUNCTION(BlueprintCallable, Category = DreamStatics)
	static void ReturnToMainMenuWithTextReason(APlayerController* PlayerCtrl, FText Reason);


	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static void CalculateFBDirection(const FVector& Velocity, const FRotator& BaseRotation, float& Angle,
	                                 bool& bIsBackward);


	static void SpawnWeaponTrailParticles(UObject* WorldContextObject, const FWeaponTrailVFX& TrailVfx,
	                                      const FVector& StartLocation, const FVector& EndLocation);

	UFUNCTION(BlueprintPure, Category = DreamStatics)
	static int32 GetWidgetZOrder(TEnumAsByte<EWidgetOrder::Type> Type);

};
