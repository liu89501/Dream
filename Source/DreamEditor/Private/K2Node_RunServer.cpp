// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_RunServer.h"
#include "PDSAsync_RunServer.h"
#include "DreamAsyncProxy.h"

UK2Node_RunServer::UK2Node_RunServer()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UDreamAsyncProxy, CreatePDSAsyncRunServer);
	ProxyFactoryClass = UDreamAsyncProxy::StaticClass();
	ProxyClass = UPDSAsync_RunServer::StaticClass();
}

FText UK2Node_RunServer::GetMenuCategory() const
{
	return FText::FromString(TEXT("PDS Async"));
}

FText UK2Node_RunServer::GetTooltipText() const
{
	return FText::FromString(TEXT("运行服务器"));
}

FText UK2Node_RunServer::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("PDS RunServer"));
}