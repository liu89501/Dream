// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserWidget.h"
#include "PSceneCapture.generated.h"


class ADMPreviewActor;
class FPreviewWorld;
class ADPreviewScene;

namespace SharedPreviewWorld
{
	extern FThreadSafeCounter GRefCount;
	
	extern FPreviewWorld* GPreviewWorld;
	
	FPreviewWorld* AllocPreviewWorld();

	void ReleasePreviewWorld();

}

class FPreviewWorld : FGCObject, FTickerObjectBase
{

public:

	FPreviewWorld();

	UWorld* GetWorld() const;

	void Initialize();

	void Destroy();

	ADPreviewScene* GetPreviewScene() const;

	virtual bool Tick(float DeltaTime) override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

private:

	bool bStopTicking;

	UWorld* PreviewWorld;

	class UDirectionalLightComponent* DirectionalLight;

	class USkyLightComponent* SkyLight;

	ADPreviewScene* PreviewScene;
};



/**
 * 
 */
UCLASS()
class UPSceneCapture : public UUserWidget
{
	GENERATED_BODY()

	UPSceneCapture(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(EditAnywhere, Category = ScenePreview)
	TSubclassOf<ADMPreviewActor> PreviewActorClass;

	UPROPERTY(EditAnywhere, Category = ScenePreview)
	FPostProcessSettings PostProcessSettings;

	UPROPERTY(EditAnywhere, Category = ScenePreview)
	FVector2D CanvasSize;

	UPROPERTY(EditAnywhere, Category = ScenePreview)
	FVector CameraLocation;

	UPROPERTY(EditAnywhere, Category = ScenePreview)
	FRotator CameraRotation;
	
	UPROPERTY(EditAnywhere, Category = ScenePreview)
	FVector PreviewPointOffset;
	
	UPROPERTY(EditAnywhere, Category = ScenePreview)
	FRotator PreviewSceneRotation;

	UPROPERTY(EditAnywhere, Category = ScenePreview)
	float CameraFOV;

	UPROPERTY(EditAnywhere, Category = ScenePreview)
	bool bUseRotate;
	
	UPROPERTY(EditAnywhere, Category = ScenePreview)
	bool bUseZoom;

public:

	UFUNCTION(BlueprintCallable, Category = ScenePreview)
	ADMPreviewActor* GetPreviewActor() const;

	UFUNCTION(BlueprintCallable, Category = ScenePreview)
	ADPreviewScene* GetPreviewSceneActor() const;

	UFUNCTION(BlueprintCallable, Category = ScenePreview)
	void UpdatePreviewScene();

protected:

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                          const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	                          const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void OnCanvasRenderTargetUpdate(UCanvas* Canvas, int32 Width, int32 Height);

private:

	FPreviewWorld* PreviewWorld;

	UPROPERTY()
	FSlateBrush PreviewBrush;

	UPROPERTY()
	class UCanvasRenderTarget2D* TextureTarget;

	UPROPERTY()
	ADMPreviewActor* PreviewActor;

	FSceneViewStateReference ViewState;

	bool bCanRotatePreviewActor;
	float RotateDelta;

	float PreviewZoomTarget;
};

