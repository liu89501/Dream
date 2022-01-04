// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameInstance.h"
#include "Engine.h"
#include "DreamType.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"
#include "PlayerGameData.h"
#include "UI/SDialog.h"
#include "UI/SSubtitle.h"

UDreamGameInstance::UDreamGameInstance()
{
}

SSubtitle* UDreamGameInstance::GetSubtitleWidget() const
{
	return Subtitle.Get();
}

void UDreamGameInstance::Init()
{
	Super::Init();

#if !UE_SERVER

	SDialog::InitInstance();
	
#endif
	
	
#if WITH_EDITOR

	FPDIStatic::Get()->Login();
	
#endif
}

void UDreamGameInstance::OnStart()
{
	Super::OnStart();

	// Client Only 
	if (UGameViewportClient* ViewportClient = GetGameViewportClient())
	{
		SAssignNew(Subtitle, SSubtitle).DisplayMode(EDisplayMode::OnePerFrame);
		ViewportClient->AddViewportWidgetContent(Subtitle.ToSharedRef(), EWidgetOrder::Subtitle);
	}
}

void UDreamGameInstance::Shutdown()
{
	Super::Shutdown();

	if (UGameViewportClient* ViewportClient = GetGameViewportClient())
	{
		if (Subtitle.IsValid())
		{
			ViewportClient->RemoveViewportWidgetContent(Subtitle.ToSharedRef());
			Subtitle.Reset();
		}
	}

#if !UE_SERVER

	SDialog::ResetInstance();

#endif
	
}
