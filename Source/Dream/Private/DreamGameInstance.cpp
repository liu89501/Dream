// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameInstance.h"

#include "DGameUserSettings.h"
#include "Engine.h"
#include "DreamType.h"
#include "MoviePlayer.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "UserWidget.h"

FString InvalidMap = TEXT("InvalidMap");

UDreamGameInstance::UDreamGameInstance()
{

}

FSurfaceImpactEffect EmptySurfaceImpactEffect = FSurfaceImpactEffect();

const FSurfaceImpactEffect& UDreamGameInstance::GetSurfaceImpactEffect(EPhysicalSurface SurfaceType)
{
	const FSurfaceImpactEffect* EffectPtr = SurfaceImpactEffects.Find(SurfaceType);
	return EffectPtr ? *EffectPtr : EmptySurfaceImpactEffect;
}

void UDreamGameInstance::LaunchDedicatedServer()
{
	if (!IsRunningDedicatedServer())
	{
		return;
	}
	

	//FPlatformProcess::ExecProcess("")
}

const FString& UDreamGameInstance::GetMapFullName(const FName& MapName) const
{
	static const FString Context = TEXT("UDreamGameInstance::GetMapFullName");
	
	if (Levels)
	{
		if (FMapInfo* MapInfo = Levels->FindRow<FMapInfo>(MapName, Context))
		{
			return MapInfo->FullName;
		}
	}

	return InvalidMap;
}

void UDreamGameInstance::Init()
{
	Super::Init();

	FPlayerDataInterfaceStatic::Startup();

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

	FPlayerDataInterfaceStatic::Shutdown();
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
