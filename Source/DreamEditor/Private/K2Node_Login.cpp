
#include "K2Node_Login.h"
#include "PlayerDataInterfaceAsyncProxy.h"
#include "PDSAsync_Login.h"

UK2Node_Login::UK2Node_Login()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UPlayerDataInterfaceAsyncProxy, CreatePDSAsyncLogin);
	ProxyFactoryClass = UPlayerDataInterfaceAsyncProxy::StaticClass();
	ProxyClass = UPDSAsync_Login::StaticClass();
}

FText UK2Node_Login::GetMenuCategory() const
{
	return FText::FromString(TEXT("PDS Async"));
}

FText UK2Node_Login::GetTooltipText() const
{
	return FText::FromString(TEXT("登录后台服务器"));
}

FText UK2Node_Login::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("PDS Login"));
}