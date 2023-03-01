// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameInstance.h"
#include "DGameplayStatics.h"
#include "DreamWidgetStatics.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"

#define LOCTEXT_NAMESPACE "DreamGameInstance"

UDreamGameInstance::UDreamGameInstance()
{
}

void UDreamGameInstance::Init()
{
	Super::Init();

	GDataInterface->OnServerConnectionLoseDelegate().BindUObject(this, &UDreamGameInstance::OnPDIConnectionLose);
	
#if WITH_EDITOR

	GDataInterface->Login();
	
#endif
	
}

void UDreamGameInstance::OnStart()
{
	Super::OnStart();
}

void UDreamGameInstance::Shutdown()
{
	Super::Shutdown();

	GDataInterface->OnServerConnectionLoseDelegate().Unbind();
}

void UDreamGameInstance::OnPDIConnectionLose()
{
	if (!IsRunningDedicatedServer())
	{
		UDGameplayStatics::ReturnToHomeWorld(this);
		UDreamWidgetStatics::PopupDialog(EDialogType::ERROR, LOCTEXT("PDI_Disconnect", "与服务器的连接已断开"), 3.f);
	}
}

#undef LOCTEXT_NAMESPACE