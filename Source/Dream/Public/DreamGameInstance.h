// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DEnemyBase.h"
#include "DreamType.h"
#include "EnumAsByte.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Engine/GameInstance.h"
#include "DreamGameInstance.generated.h"

/**
 *
 */
UCLASS()
class DREAM_API UDreamGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UDreamGameInstance();

	static FString InvalidMap;

	/**
	 * 物品品级配置项
	 */
	UPROPERTY(EditAnywhere, Category = "Settings|Settings")
	TMap<EPropsQuality, FQualityInfo> QualitySettings;
	
	/**
	 * 加载游戏时显示的界面
	 */
	UPROPERTY(EditAnywhere, Category = "Settings|Settings")
	TSubclassOf<class UUserWidget> LoadingScreenWidgetClass;

	/**
	 * 	关卡数据
	 */
	UPROPERTY(EditAnywhere, Category = "Settings|Settings")
	UDataTable* Levels;

	UPROPERTY(EditAnywhere, Category = "Settings|Settings")
	TMap<TEnumAsByte<EPhysicalSurface>, FSurfaceImpactEffect> SurfaceImpactEffects;


	
	const FString& GetMapFullName(const FName& MapName) const;

	const FSurfaceImpactEffect& GetSurfaceImpactEffect(EPhysicalSurface SurfaceType);

protected:

	virtual void Init() override;

	virtual void Shutdown() override;
	
	UFUNCTION()
	virtual void OnPreLoadMap(const FString& MapName);

	UFUNCTION()
    virtual void OnPostLoadMap(UWorld* LoadedWorld);

private:

	UPROPERTY()
	class UUserWidget* LoadingScreenWidget;

};