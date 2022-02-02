// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "DreamWidgetType.h"
#include "ShootWeapon.h"
#include "UI/Style/DialogSlateWidgetStyle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamWidgetStatics.generated.h"

USTRUCT(BlueprintType)
struct FModuleExtraData
{
	GENERATED_BODY()

	FModuleExtraData() : Category(EModuleCategory::C1)
	{
		
	}

	UPROPERTY(BlueprintReadOnly, Category=ExtraData)
	EModuleCategory Category;
};

USTRUCT(BlueprintType)
struct FPlayerStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FText PlayerName;
	
	UPROPERTY(BlueprintReadOnly)
	FText Kills;
	
	UPROPERTY(BlueprintReadOnly)
	FText TotalDamage;
};

USTRUCT(BlueprintType)
struct FWeaponExtraData
{
	GENERATED_BODY()

	FWeaponExtraData()
		: RateOfFire(0),
		  Magazine(0),
		  FireMode(EFireMode::Accumulation),
		  WeaponType(EWeaponType::Shotgun)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category=ExtraData)
	int32 RateOfFire;

	UPROPERTY(BlueprintReadOnly, Category=ExtraData)
	int32 Magazine;

	UPROPERTY(BlueprintReadOnly, Category=ExtraData)
	EFireMode FireMode;

	UPROPERTY(BlueprintReadOnly, Category=ExtraData)
	EWeaponType WeaponType;
};


/**
 * 
 */
UCLASS()
class DREAM_API UDreamWidgetStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category = "DreamWidget")
    static UWItemEquipment* MakeWEquipmentFromPW(UObject* WorldContextObject, const FPlayerWeapon& PW);
	
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category = "DreamWidget")
    static UWItemEquipment* MakeWEquipmentFromPM(UObject* WorldContextObject, const FPlayerModule& PM);

	/**
	* 	替换widget中的子项
	*/
	static void ReplaceWidgetChildAt(class UPanelWidget* ParentWidget, int32 ChildIndex, class UWidget* NewWidget);

	UFUNCTION(BlueprintPure, Category = "DreamWidget")
    static int32 GetWidgetZOrder(TEnumAsByte<EWidgetOrder::Type> Type);

	/** 聚焦到viewport */
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf="Widget", HidePin="Widget"), Category = "DreamWidget")
    static void FocusOnViewport(UWidget* Widget);

	/**
	 * 计算弹窗的位置, 基于鼠标指针的位置
	 */
	UFUNCTION(BlueprintPure, Category = "DreamWidget", meta=( WorldContext="WorldContextObject" ))
	static FVector2D CalculationPopupPosition(UObject* WorldContextObject, const FVector2D& WidgetSize, const FVector2D& PointerSize);

	UFUNCTION(BlueprintCallable, Category = "DreamWidget")
	static void PopupDialog(EDialogType DialogType, FText Content, float DisplayTime = 2.f);

	UFUNCTION(BlueprintCallable, Category = "DreamWidget")
	static void DismissDialog();
    	
    UFUNCTION(BlueprintPure, Category = "DreamWidget")
    static const FPropsInfo& GetPropsInfoByItemGuid(int32 ItemGuid);
	
    UFUNCTION(BlueprintPure, Category = "DreamWidget")
    static UClass* GetItemClassByGuid(int32 ItemGuid);

	UFUNCTION(BlueprintPure, Category = "DreamWidget")
	static FModuleExtraData GetModuleExtraData(UClass* ModuleClass);
	
	UFUNCTION(BlueprintPure, Category = "DreamWidget")
	static FWeaponExtraData GetWeaponExtraData(UClass* WeaponClass);

	UFUNCTION(BlueprintCallable, meta=( WorldContext="WorldContextObject" ), Category = "DreamWidget")
	static TArray<FPlayerStatistics> GetPlayerStatistics(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "DreamWidget")
	static void AddGuideActorToLocalPlayer(AActor* GuideActor);
	
	UFUNCTION(BlueprintCallable, Category = "DreamWidget")
	static void RemoveGuideActorFromLocalPlayer(AActor* GuideActor);

	UFUNCTION(BlueprintCallable, Category = "DreamWidget")
	static void DisplaySubtitle(FText Text);

	UFUNCTION(BlueprintCallable, Category = "DreamWidget")
	static void AddWidgetToViewport(UUserWidget* Widget, TEnumAsByte<EWidgetOrder::Type> ZOrder);

	/** 将 Weight (0 ~ 1) 转换成百分比的文本 */
	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "Percentage", BlueprintAutocast), Category = "DreamWidget")
	static FText WeightToPercentageText(float Weight);


	UFUNCTION(BlueprintPure, Category = "DreamWidget|Settings")
	static FText GetWeaponTypeName(EWeaponType WeaponType);


	/** item Handle */

	UFUNCTION(BlueprintPure, Category="DreamWidget|Item")
    static const FPropsInfo& GetPropsInfo(const FItemHandle& ItemHandle);


	UFUNCTION(BlueprintPure, Category="DreamWidget|Item")
	static const FQualityInfo& GetQualityInfo(EPropsQuality Quality);
	
private:
	
	template<class Class>
	static Class* GetGameInstance(UObject* WorldContextObject);
};

