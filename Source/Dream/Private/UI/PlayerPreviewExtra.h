// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserWidget.h"
#include "PlayerPreviewExtra.generated.h"

class ADItemPreview;

/**
 * 
 */
UCLASS()
class UPlayerPreviewExtra : public UUserWidget
{
	GENERATED_BODY()

public:
	UPlayerPreviewExtra(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(EditAnywhere, Category = PlayerPreview)
	FVector2D CanvasSize;

	UPROPERTY(EditAnywhere, Category = PlayerPreview)
	FVector CameraLocation;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlayerPreview)
	FRotator CameraRotation;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlayerPreview)
	float CameraFOV;

	UPROPERTY(EditAnywhere, Meta = (MetaClass="World"), Category = PlayerPreview)
	FSoftObjectPath PreviewWorldAsset;

public:

	UFUNCTION(BlueprintPure, Category = PlayerPreview)
	UTexture* GetPreviewTexture() const;

	UFUNCTION(BlueprintPure, Category = PlayerPreview)
	ADItemPreview* GetPreviewActor() const;

protected:

	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION()
    void OnCanvasRenderTargetUpdate(UCanvas* Canvas, int32 Width, int32 Height);

private:

	UPROPERTY()
	ADItemPreview* PreviewActor;

	UPROPERTY()
	UWorld* PreviewWorld;

	UPROPERTY()
	class UCanvasRenderTarget2D* TextureTarget;
	
	FSceneViewStateReference ViewState;
};
