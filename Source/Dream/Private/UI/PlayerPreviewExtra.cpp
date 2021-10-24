// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PlayerPreviewExtra.h"
#include "DItemPreview.h"
#include "EngineModule.h"
#include "EngineUtils.h"
#include "LegacyScreenPercentageDriver.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"

UPlayerPreviewExtra::UPlayerPreviewExtra(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CameraLocation = FVector(-100, 0, -140);
	CameraFOV = 60.f;
	CanvasSize = FVector2D(1024.f, 1024.f);
}

UTexture* UPlayerPreviewExtra::GetPreviewTexture() const
{
	return TextureTarget;
}

ADItemPreview* UPlayerPreviewExtra::GetPreviewActor() const
{
	return PreviewActor;
}

void UPlayerPreviewExtra::NativeOnInitialized()
{
	UPackage* WorldPackage = FindPackage(nullptr, *PreviewWorldAsset.GetLongPackageName());

	if (WorldPackage == nullptr)
	{
		WorldPackage = LoadPackage(nullptr, *PreviewWorldAsset.GetLongPackageName(), LOAD_None);
	}

	PreviewWorld = UWorld::FindWorldInPackage(WorldPackage);
	checkf(PreviewWorld, TEXT("FindWorldInPackage Failure"));

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(IsRunningGame() ? EWorldType::GamePreview : EWorldType::EditorPreview);

	WorldContext.SetCurrentWorld(PreviewWorld);
	PreviewWorld->WorldType = WorldContext.WorldType;

	if (!PreviewWorld->bIsWorldInitialized)
	{
		PreviewWorld->InitWorld();
	}

	for (ADItemPreview* ItemPreview : TActorRange<ADItemPreview>(PreviewWorld))
	{
		PreviewActor = ItemPreview;
		break;
	}
	
	ViewState.Allocate();
    
	TextureTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(GetWorld(), UCanvasRenderTarget2D::StaticClass(), CanvasSize.X, CanvasSize.Y);
	TextureTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &UPlayerPreviewExtra::OnCanvasRenderTargetUpdate);
	TextureTarget->UpdateResourceImmediate();

	Super::NativeOnInitialized();
}

void UPlayerPreviewExtra::NativeDestruct()
{
	Super::NativeDestruct();

	TextureTarget->OnCanvasRenderTargetUpdate.Clear();

	ViewState.Destroy();

	if (PreviewWorld)
	{
		PreviewWorld->DestroyWorld(true);
		GEngine->DestroyWorldContext(PreviewWorld);
		PreviewWorld = nullptr;
		GEngine->ForceGarbageCollection(true);
	}
}

void UPlayerPreviewExtra::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (PreviewWorld)
	{
		PreviewWorld->Tick(LEVELTICK_All, InDeltaTime);
	}

	if (TextureTarget)
	{
		TextureTarget->FastUpdateResource();
	}
}

void UPlayerPreviewExtra::OnCanvasRenderTargetUpdate(UCanvas* Canvas, int32 Width, int32 Height)
{
	FVector2D AbsoluteSize = GetTickSpaceGeometry().GetAbsoluteSize();
	if (AbsoluteSize.IsNearlyZero())
	{
		return;
	}
	
	FEngineShowFlags ShowFlags(ESFIM_Game);
	ShowFlags.SetMotionBlur(false);
	
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		TextureTarget->GameThread_GetRenderTargetResource(), PreviewWorld->Scene, ShowFlags)
		.SetRealtimeUpdate(true)
		.SetResolveScene(true));

	FTransform Transform(CameraRotation);
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

	FSceneViewInitOptions SceneViewInitOptions;
	SceneViewInitOptions.SetViewRectangle(FIntRect(0, 0, Width, Height));
	SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
	SceneViewInitOptions.ViewFamily = &ViewFamily;
	SceneViewInitOptions.ViewOrigin = CameraLocation;
	SceneViewInitOptions.SceneViewStateInterface = ViewState.GetReference();
	SceneViewInitOptions.LODDistanceFactor = 1.f;
	SceneViewInitOptions.BackgroundColor = FLinearColor::Black;
	SceneViewInitOptions.OverrideFarClippingPlaneDistance = 0.f;
	SceneViewInitOptions.ProjectionMatrix = ProjectionMatrix;

	SceneViewInitOptions.CursorPos = FIntPoint(-1, -1);
	SceneViewInitOptions.WorldToMetersScale = PreviewWorld->GetWorldSettings()->WorldToMeters;

	FSceneView* View = new FSceneView(SceneViewInitOptions);
	View->bIsSceneCapture = true;
	View->SetupAntiAliasingMethod();

	ViewFamily.Views.Add(View);

	View->StartFinalPostprocessSettings(CameraLocation);
	View->EndFinalPostprocessSettings(SceneViewInitOptions);

	ViewFamily.SceneCaptureSource = SCS_FinalColorHDR;
	ViewFamily.EngineShowFlags.ScreenPercentage = false;
	ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, 1.0f, false));

	GetRendererModule().BeginRenderingViewFamily(Canvas->Canvas, &ViewFamily);
}
