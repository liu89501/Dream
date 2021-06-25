
#include "K2Node_SwitchWeapon.h"
#include "PDSAsync_SwitchWeapon.h"
#include "PlayerDataInterfaceAsyncProxy.h"

UK2Node_SwitchWeapon::UK2Node_SwitchWeapon()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UPlayerDataInterfaceAsyncProxy, CreatePDSAsyncSwitchWeapon);
	ProxyFactoryClass = UPlayerDataInterfaceAsyncProxy::StaticClass();
	ProxyClass = UPDSAsync_SwitchWeapon::StaticClass();
}

FText UK2Node_SwitchWeapon::GetMenuCategory() const
{
	return FText::FromString(TEXT("PDS Async"));
}

FText UK2Node_SwitchWeapon::GetTooltipText() const
{
	return FText::FromString(TEXT("切换武器"));
}

FText UK2Node_SwitchWeapon::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("PDS SwitchWeapon"));
}