
#include "K2Node_GetWeapons.h"
#include "PDSAsync_GetWeapons.h"
#include "PlayerDataInterfaceAsyncProxy.h"

UK2Node_GetWeapons::UK2Node_GetWeapons()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UPlayerDataInterfaceAsyncProxy, CreatePDSAsyncGetWeapons);
	ProxyFactoryClass = UPlayerDataInterfaceAsyncProxy::StaticClass();
	ProxyClass = UPDSAsync_GetWeapons::StaticClass();
}

FText UK2Node_GetWeapons::GetMenuCategory() const
{
	return FText::FromString(TEXT("PDS Async"));
}

FText UK2Node_GetWeapons::GetTooltipText() const
{
	return FText::FromString(TEXT("获取后台服务器玩家武器信息"));
}

FText UK2Node_GetWeapons::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("PDS GetWeapons"));
}