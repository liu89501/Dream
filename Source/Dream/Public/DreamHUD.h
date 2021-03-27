// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DreamHUD.generated.h"

USTRUCT(BlueprintType)
struct DREAM_API FMaterialWidget
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D Size;
};

USTRUCT(BlueprintType)
struct DREAM_API FTextureWidget
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture* Texture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D Size;
};

UCLASS()
class ADreamHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADreamHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

protected:

	virtual void PostInitializeComponents() override;

public:

	/** 击中敌人提示标识的持续时间 */
	UPROPERTY(EditAnywhere, Category = FPS)
	float HitRemindDuration;

	/** 击中提示的纹理 */
	UPROPERTY(EditAnywhere, Category = FPS)
	UTexture2D* HitRemindTexture;

	/** 小地图 */
	UPROPERTY(EditAnywhere, Category = FPS)
	UMaterialInstance* MinimalMap;

private:

	bool bShowHitRemindTips;

	float CrosshairSpreadCount;

	FLinearColor HitRemindColor;

};
