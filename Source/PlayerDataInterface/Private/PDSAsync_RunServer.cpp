// Fill out your copyright notice in the Description page of Project Settings.


#include "PDSAsync_RunServer.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"

void UPDSAsync_RunServer::Init(const FRunServerParameter& Parameter)
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnRunServerComplete.AddUObject(this, &UPDSAsync_RunServer::OnLoadCompleted);
		PDS->RunServer(Parameter);
	}
}

void UPDSAsync_RunServer::OnLoadCompleted(const FString& ServerAddr, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(ServerAddr);
	}
	else
	{
		OnFail.Broadcast(ServerAddr);
	}

	FPlayerDataInterfaceModule::Get()->OnRunServerComplete.Remove(Handle);
}
