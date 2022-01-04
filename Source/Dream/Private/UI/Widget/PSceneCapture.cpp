// Fill out your copyright notice in the Description page of Project Settings.

#include "PSceneCapture.h"
#include "DItemPreview.h"
#include "DPreviewScene.h"
#include "EngineModule.h"
#include "EngineUtils.h"
#include "LegacyScreenPercentageDriver.h"
#include "PreviewSceneSettings.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"


FPreviewWorld::FPreviewWorld(): bStopTicking(false), PreviewWorld(nullptr), DirectionalLight(nullptr),
                                SkyLight(nullptr), PreviewScene(nullptr)
{
}

UWorld* FPreviewWorld::GetWorld() const
{
	return PreviewWorld;
}

void FPreviewWorld::Initialize()
{
	PreviewWorld = UWorld::CreateWorld(EWorldType::GamePreview, true);
	GEngine->CreateNewWorldContext(EWorldType::GamePreview).SetCurrentWorld(PreviewWorld);
	PreviewWorld->InitializeActorsForPlay(FURL());

	UPreviewSceneSettings* SceneSettings = UPreviewSceneSettings::Get();

	if (SceneSettings->bUseDirectionalLight)
	{
		DirectionalLight = NewObject<UDirectionalLightComponent>(GetTransientPackage());
		DirectionalLight->Intensity = SceneSettings->LightBrightness;
		DirectionalLight->LightColor = SceneSettings->LightColor;
		DirectionalLight->RegisterComponentWithWorld(PreviewWorld);
	}

	if (SceneSettings->bUseSkyLight)
	{
		SkyLight = NewObject<USkyLightComponent>(GetTransientPackage());
		SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
		SkyLight->SkyDistanceThreshold = SceneSettings->SkyDistanceThreshold;
		SkyLight->Intensity = SceneSettings->SkyLightIntensity;
		SkyLight->Mobility = EComponentMobility::Movable;
		SkyLight->RegisterComponentWithWorld(PreviewWorld);
	}

	if (SceneSettings->PreviewSceneClass.IsValid())
	{
		if (UClass* PreviewSceneClass = SceneSettings->PreviewSceneClass.TryLoadClass<ADPreviewScene>())
		{
			PreviewScene = PreviewWorld->SpawnActor<ADPreviewScene>(PreviewSceneClass);
		}
	}
}

void FPreviewWorld::Destroy()
{
	//UE_LOG(LogTemp, Error, TEXT("DestroyPreviewWorld"));
	
	bStopTicking = false;
	PreviewWorld->DestroyWorld(true);
	GEngine->DestroyWorldContext(PreviewWorld);
	PreviewWorld = nullptr;
	GEngine->ForceGarbageCollection(true);
}

ADPreviewScene* FPreviewWorld::GetPreviewScene() const
{
	return PreviewScene;
}

bool FPreviewWorld::Tick(float DeltaTime)
{
	if (bStopTicking)
	{
		return false;
	}
	
	if (PreviewWorld)
	{
		PreviewWorld->Tick(LEVELTICK_ViewportsOnly, DeltaTime);
		USkyLightComponent::UpdateSkyCaptureContents(PreviewWorld);
	}
	
	return true;
}

void FPreviewWorld::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PreviewWorld);
	Collector.AddReferencedObject(DirectionalLight);
	Collector.AddReferencedObject(SkyLight);
	Collector.AddReferencedObject(PreviewScene);
}

FThreadSafeCounter SharedPreviewWorld::GRefCount = FThreadSafeCounter(0);

FPreviewWorld* SharedPreviewWorld::GPreviewWorld = nullptr;

FPreviewWorld* SharedPreviewWorld::AllocPreviewWorld()
{
	if (GPreviewWorld == nullptr)
	{
		GPreviewWorld = new FPreviewWorld();
		GPreviewWorld->Initialize();
		GRefCount.Reset();
	}

	GRefCount.Increment();

	//UE_LOG(LogTemp, Error, TEXT("AllocPreviewWorld %d"), GRefCount.GetValue());

	return GPreviewWorld;
}

void SharedPreviewWorld::ReleasePreviewWorld()
{
	if (GRefCount.Decrement() == 0)
	{
		if (GPreviewWorld)
		{
			GPreviewWorld->Destroy();

			delete GPreviewWorld;
			
			GPreviewWorld = nullptr;
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("ReleasePreviewWorld %d"), GRefCount.GetValue());
}

UPSceneCapture::UPSceneCapture(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	CanvasSize(FVector2D(1024.f, 1024.f)),
	CameraLocation(FVector(-100, 0, -140)),
	CameraFOV(60.f),
	bCanRotatePreviewActor(false),
	RotateDelta(0.f)
{
	PreviewZoomTarget = CameraFOV;
}

ADItemPreview* UPSceneCapture::GetPreviewActor() const
{
	return PreviewActor;
}

ADPreviewScene* UPSceneCapture::GetPreviewSceneActor() const
{
	return PreviewWorld->GetPreviewScene();
}

void UPSceneCapture::UpdatePreviewScene()
{
	ADPreviewScene* PreviewSceneActor = PreviewWorld->GetPreviewScene();
	PreviewSceneActor->PreviewActorPoint->SetRelativeLocation(PreviewPointOffset);
	PreviewSceneActor->SetActorRotation(PreviewSceneRotation);
}

FReply UPSceneCapture::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bUseRotate && bCanRotatePreviewActor)
	{
		const FVector2D CursorDelta = InMouseEvent.GetCursorDelta();

		RotateDelta += CursorDelta.X * -1;

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UPSceneCapture::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (bUseRotate)
	{
		bCanRotatePreviewActor = false;
	}
}

FReply UPSceneCapture::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bUseRotate)
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bCanRotatePreviewActor = true;

			return FReply::Handled();
		}
	}
	
	return FReply::Unhandled();
}

FReply UPSceneCapture::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bUseRotate)
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bCanRotatePreviewActor = false;
			return FReply::Handled();
		}
	}
	
	return FReply::Unhandled();
}

FReply UPSceneCapture::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bUseZoom)
	{
		float WheelDelta = InMouseEvent.GetWheelDelta();
		PreviewZoomTarget = FMath::Clamp(PreviewZoomTarget + WheelDelta, 35.f, 70.f);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void UPSceneCapture::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PreviewWorld = SharedPreviewWorld::AllocPreviewWorld();

	if (PreviewActorClass)
	{
		PreviewActor = PreviewWorld->GetWorld()->SpawnActor<ADItemPreview>(PreviewActorClass);
		
		ADPreviewScene* PreviewSceneActor = PreviewWorld->GetPreviewScene();
		PreviewActor->AttachToComponent(PreviewSceneActor->PreviewActorPoint, FAttachmentTransformRules::KeepRelativeTransform);

		UpdatePreviewScene();

		PreviewActor->DispatchBeginPlay();
	}

	ViewState.Allocate();
    
	TextureTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(GetWorld(), UCanvasRenderTarget2D::StaticClass(), CanvasSize.X, CanvasSize.Y);
	TextureTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &UPSceneCapture::OnCanvasRenderTargetUpdate);
	TextureTarget->UpdateResourceImmediate();

	PreviewBrush.SetResourceObject(TextureTarget);
}

void UPSceneCapture::NativeDestruct()
{
	Super::NativeDestruct();

	TextureTarget->OnCanvasRenderTargetUpdate.Clear();

	TextureTarget = nullptr;

	ViewState.Destroy();

	PreviewActor->Destroy();
	PreviewActor = nullptr;

	SharedPreviewWorld::ReleasePreviewWorld();
}

int32 UPSceneCapture::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
    const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (PreviewActor)
	{
		FPaintGeometry Geometry = AllottedGeometry.ToPaintGeometry(
                    AllottedGeometry.GetLocalSize(),
                    FSlateLayoutTransform(),
                    FSlateRenderTransform());

		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry, &PreviewBrush);
	}
	
	return LayerId;
}

void UPSceneCapture::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bCanRotatePreviewActor)
	{
		USceneComponent* Component = PreviewActor->GetRootComponent();
		FRotator Rotation = Component->GetRelativeRotation();
		FRotator TargetRotation = FRotator(Rotation.Pitch, RotateDelta, Rotation.Roll);
		Component->SetRelativeRotation(FMath::RInterpTo(Rotation, TargetRotation, InDeltaTime, 14.f));
	}

	CameraFOV = FMath::FInterpTo(CameraFOV, PreviewZoomTarget, InDeltaTime, 14.f);

	if (TextureTarget)
	{
		TextureTarget->FastUpdateResource();
	}
}

void UPSceneCapture::OnCanvasRenderTargetUpdate(UCanvas* Canvas, int32 Width, int32 Height)
{
	FVector2D AbsoluteSize = GetTickSpaceGeometry().GetAbsoluteSize();
	if (AbsoluteSize.IsNearlyZero())
	{
		return;
	}
	
	FEngineShowFlags ShowFlags(ESFIM_Game);
	ShowFlags.SetMotionBlur(false);
	
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		TextureTarget->GameThread_GetRenderTargetResource(), PreviewWorld->GetWorld()->Scene, ShowFlags)
		.SetRealtimeUpdate(true)
		.SetResolveScene(true));

	
	FTransform Transform(PreviewSceneRotation + CameraRotation);
	FMatrix ViewRotationMatrix = Transform.ToInverseMatrixWithScale();

	// swap axis st. x=z,y=x,z=y (unreal coord space) so that z is up
	ViewRotationMatrix = ViewRotationMatrix * FMatrix(
        FPlane(0, 0, 1, 0),
        FPlane(1, 0, 0, 0),
        FPlane(0, 1, 0, 0),
        FPlane(0, 0, 0, 1));

	FReversedZPerspectiveMatrix ProjectionMatrix(
                FMath::Max(1.f, CameraFOV) * PI / 360.0f,
                AbsoluteSize.X / AbsoluteSize.Y,
                1.0f,
                GNearClippingPlane
                );

	ADPreviewScene* PreviewScene = PreviewWorld->GetPreviewScene();

	FSceneViewInitOptions SceneViewInitOptions;
	SceneViewInitOptions.SetViewRectangle(FIntRect(0, 0, Width, Height));
	SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
	SceneViewInitOptions.ViewFamily = &ViewFamily;
	SceneViewInitOptions.ViewOrigin = PreviewSceneRotation.RotateVector(PreviewPointOffset + CameraLocation);
	SceneViewInitOptions.SceneViewStateInterface = ViewState.GetReference();
	SceneViewInitOptions.LODDistanceFactor = 1.f;
	SceneViewInitOptions.ViewActor = PreviewScene;
	SceneViewInitOptions.BackgroundColor = FLinearColor::Black;
	SceneViewInitOptions.OverrideFarClippingPlaneDistance = 0.f;
	SceneViewInitOptions.ProjectionMatrix = ProjectionMatrix;

	TSet<FPrimitiveComponentId> ShowOnlyPrimitives;

	// 预览的场景组件
	TArray<USceneComponent*> SceneChildren;
	PreviewScene->SceneActor->GetChildrenComponents(true, SceneChildren);
	for (USceneComponent* ChildComponent : SceneChildren)
	{
		if (UPrimitiveComponent* ChildPrimitiveComp = Cast<UPrimitiveComponent>(ChildComponent))
		{
			ShowOnlyPrimitives.Add(ChildPrimitiveComp->ComponentId);
		}
	}

	// 预览actor
	TArray<USceneComponent*> PreviewChildren;
	PreviewActor->GetRootComponent()->GetChildrenComponents(true, PreviewChildren);
	for (USceneComponent* ChildComponent : PreviewChildren)
	{
		if (UPrimitiveComponent* ChildPrimitiveComp = Cast<UPrimitiveComponent>(ChildComponent))
		{
			ShowOnlyPrimitives.Add(ChildPrimitiveComp->ComponentId);
		}
	}

	if (ShowOnlyPrimitives.Num() > 0)
	{
		SceneViewInitOptions.ShowOnlyPrimitives = ShowOnlyPrimitives;
	}

	SceneViewInitOptions.CursorPos = FIntPoint(-1, -1);
	SceneViewInitOptions.WorldToMetersScale = PreviewWorld->GetWorld()->GetWorldSettings()->WorldToMeters;

	FSceneView* View = new FSceneView(SceneViewInitOptions);
	View->bIsSceneCapture = true;
	View->SetupAntiAliasingMethod();

	ViewFamily.Views.Add(View);

	View->StartFinalPostprocessSettings(CameraLocation);
	View->OverridePostProcessSettings(PostProcessSettings, 1.f);
	View->EndFinalPostprocessSettings(SceneViewInitOptions);

	ViewFamily.SceneCaptureSource = SCS_FinalColorHDR;
	ViewFamily.EngineShowFlags.ScreenPercentage = false;
	ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, 1.0f, false));

	GetRendererModule().BeginRenderingViewFamily(Canvas->Canvas, &ViewFamily);
}
