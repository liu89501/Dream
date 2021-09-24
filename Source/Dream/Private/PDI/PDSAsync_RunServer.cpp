// Fill out your copyright notice in the Description page of Project Settings.


#include "PDI/PDSAsync_RunServer.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_RunServer* UPDSAsync_RunServer::PDI_GetServer(UObject* WorldContextObject, const FRunServerParameter& Parameter)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_RunServer* PDSI = NewObject<UPDSAsync_RunServer>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [PDSI, Parameter]
        {
			FGetServerComplete Delegate;
			Delegate.BindUObject(PDSI, &UPDSAsync_RunServer::OnLoadCompleted);
			FPlayerDataInterfaceStatic::Get()->RunServer(Parameter, Delegate);
			
        }, 0.001f, false);
		
		return PDSI;
	}
	
	return nullptr;
}

void UPDSAsync_RunServer::OnLoadCompleted(const FString& ServerAddr, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(ServerAddr);
	}
	else
	{
		OnFailure.Broadcast(ServerAddr);
	}
}
