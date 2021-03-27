// Fill out your copyright notice in the Description page of Project Settings.


#include "PDSAsync_RunServer.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"

void UPDSAsync_RunServer::Init(const FRunServerParameter& Parameter)
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnRunServerComplete.AddUObject(this, &UPDSAsync_RunServer::OnLoadCompleted);
		PDS->RunServer(Parameter);
	}
}

void UPDSAsync_RunServer::OnLoadCompleted(const FString& ServerAddr, const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(ServerAddr);
	}
	else
	{
		OnFail.Broadcast(ServerAddr);
	}

	FPlayerDataStoreModule::Get()->OnRunServerComplete.Remove(Handle);
}