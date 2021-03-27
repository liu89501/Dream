// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_PayItem.h"
#include "DreamAsyncProxy.h"
#include "PDSAsync_PayItem.h"

UK2Node_PayItem::UK2Node_PayItem()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UDreamAsyncProxy, CreatePDSAsyncPayItem);
	ProxyFactoryClass = UDreamAsyncProxy::StaticClass();
	ProxyClass = UPDSAsync_PayItem::StaticClass();
}

FText UK2Node_PayItem::GetMenuCategory() const
{
	return FText::FromString(TEXT("PDS Async"));
}

FText UK2Node_PayItem::GetTooltipText() const
{
	return FText::FromString(TEXT("购买物品"));
}

FText UK2Node_PayItem::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("PDS PayItem"));
}