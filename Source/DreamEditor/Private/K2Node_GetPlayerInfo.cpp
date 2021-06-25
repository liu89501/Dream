
#include "K2Node_GetPlayerInfo.h"
#include "PlayerDataInterfaceAsyncProxy.h"
#include "PDSAsync_GetPlayerInfo.h"

UK2Node_GetPlayerInfo::UK2Node_GetPlayerInfo()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UPlayerDataInterfaceAsyncProxy, CreatePDSAsyncGetPlayerInfo);
	ProxyFactoryClass = UPlayerDataInterfaceAsyncProxy::StaticClass();
	ProxyClass = UPDSAsync_GetPlayerInfo::StaticClass();
}

FText UK2Node_GetPlayerInfo::GetMenuCategory() const
{
	return FText::FromString(TEXT("PDS Async"));
}

FText UK2Node_GetPlayerInfo::GetTooltipText() const
{
	return FText::FromString(TEXT("获取玩家信息"));
}

FText UK2Node_GetPlayerInfo::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("PDS GetPlayerInfo"));
}