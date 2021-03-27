// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DAIModuleComponent.h"
#include "DreamType.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DGameplayStatics.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API UDGameplayStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure, Category = DreamStatics)
    static TSubclassOf<class AShootWeapon> LoadWeaponClass(FString QualifiedName);

    UFUNCTION(BlueprintPure, Category = DreamStatics)
    static FString GetClassPathName(TSubclassOf<UObject> ObjectClass);

    UFUNCTION(BlueprintCallable, Category = DreamStatics)
    static bool GetClassPropsInfo(const FString& ClassString, FPropsInfo& PropsInfo);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
    static bool GetQualityInfo(UObject* WorldContextObject, EPropsQuality Quality, FQualityInfo& QualityInfo);

    UFUNCTION(BlueprintPure, Category = DreamStatics)
    static FLinearColor ParseColorFromString(const FString& ColorString);

    UFUNCTION(BlueprintCallable, Category = DreamStatics)
    static bool ContainsActionKey(class APlayerController* PlayerController, FKey Key, FName ActionName);

    UFUNCTION(BlueprintCallable, Category = DreamStatics)
    static bool SetFocus(class UWidget* Widget);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
    static void ServerTravel(UObject* WorldContextObject, const FString& URL, bool bAbsolute);

    /* DATE 转换为 文本 */
    UFUNCTION(BlueprintPure, Category = DreamStatics)
    static FText ToTimeText(int32 TotalSeconds);

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
     * 获取默认的重力值
     */
    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = DreamStatics)
    static float GetDefaultGravityZ(UObject* WorldContextObject);


    /**
     * 	替换widget中的子项
     */
    static void ReplaceWidgetChildAt(class UPanelWidget* ParentWidget, int32 ChildIndex, class UWidget* NewWidget);

    /**
     *  创建伤害widget
     */
    UE_DEPRECATED(4.25, "暂时弃用")
    UFUNCTION(BlueprintCallable, Category = "DreamStatics|Abilities")
    static void SpawnDamageWidgets(AActor* TargetActor, const struct FGameplayCueParameters& Parameters, bool bHealthSteal);

    static struct FDreamGameplayEffectContext* MakeDreamEffectContextHandle(
        AActor* SourceActor, UCurveFloat* DamageFalloffCurve, const FHitResult& Hit, const FVector& Origin);


    /**
     *  获取Actor玩家控制器, 如果没找到会返回null
     */
    UFUNCTION(BlueprintPure, Category = DreamStatics)
    static APlayerController* GetActorPlayerController(AActor* Actor);

    /** 返回到主菜单界面 */
    UFUNCTION(BlueprintCallable, Category = DreamStatics)
    static void ReturnToMainMenuWithTextReason(APlayerController* PlayerCtrl, FText Reason);
};
