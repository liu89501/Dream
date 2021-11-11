// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameInstance.h"
#include "Engine.h"
#include "DreamType.h"
#include "MoviePlayer.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"
#include "PlayerGameData.h"
#include "UserWidget.h"

UDreamGameInstance::UDreamGameInstance()
{

}

FSurfaceImpactEffect EmptySurfaceImpactEffect = FSurfaceImpactEffect();

const FSurfaceImpactEffect& UDreamGameInstance::GetSurfaceImpactEffect(EPhysicalSurface SurfaceType)
{
	const FSurfaceImpactEffect* EffectPtr = SurfaceImpactEffects.Find(SurfaceType);
	return EffectPtr ? *EffectPtr : EmptySurfaceImpactEffect;
}

void UDreamGameInstance::Init()
{
	Super::Init();

	FPDIStatic::Startup();

#if WITH_EDITOR

	FPDIStatic::Get()->Login();
	
#endif


	if (!IsRunningDedicatedServer())
	{
		LoadingScreenWidget = CreateWidget(this, LoadingScreenWidgetClass);
		FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UDreamGameInstance::OnPreLoadMap);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UDreamGameInstance::OnPostLoadMap);
	}
}

void UDreamGameInstance::Shutdown()
{
	Super::Shutdown();

	FPDIStatic::Shutdown();
}

void UDreamGameInstance::OnPreLoadMap(const FString& MapName)
{
	/*if (MapName == TEXT("/Game/Maps/MainMenuUI"))
	{
		return;
	}*/

	FLoadingScreenAttributes Attr;
	Attr.WidgetLoadingScreen = LoadingScreenWidget->TakeWidget();
	GetMoviePlayer()->SetupLoadingScreen(Attr);
}

void UDreamGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	GetMoviePlayer()->StopMovie();
}
