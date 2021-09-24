// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserWidget.h"
#include "PlayerPreview.generated.h"

class ADItemPreview;

/**
 * 
 */
UCLASS()
class UPlayerPreview : public UUserWidget
{
	GENERATED_BODY()

public:

	UPlayerPreview(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(EditAnywhere, Category=PlayerPreview)
	TSubclassOf<ADItemPreview> PreviewActorClass;
	
	UPROPERTY(EditAnywhere, Category = PlayerPreview)
	FPostProcessSettings PostProcessSettings;
	
	UPROPERTY(EditAnywhere, Category = PlayerPreview)
	FVector2D CanvasSize;

	UPROPERTY(EditAnywhere, Category = PlayerPreview)
	FVector CameraLocation;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlayerPreview)
	FRotator CameraRotation;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlayerPreview)
	float CameraFOV;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn), Category = PlayerPreview)
	FUniqueNetIdRepl UniqueNetId;

	UPROPERTY(BlueprintReadOnly, Category = PlayerPreview)
	ADItemPreview* PreviewActor;

public:

	UFUNCTION(BlueprintPure, Category = PlayerPreview)
	UTexture* GetPreviewTexture() const;

	UFUNCTION(BlueprintCallable, Category = PlayerPreview)
	AActor* SpawnPreviewActor(TSubclassOf<AActor> ActorClass);

protected:

	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION()
    void OnCanvasRenderTargetUpdate(UCanvas* Canvas, int32 Width, int32 Height);

private:

	UPROPERTY()
	UWorld* PreviewWorld;

	UPROPERTY()
	class UCanvasRenderTarget2D* TextureTarget;
	
	FSceneViewStateReference ViewState;
};
