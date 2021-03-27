
#include "K2Node_GetStoreItems.h"
#include "DreamAsyncProxy.h"
#include "PDSAsync_GetStoreItems.h"

UK2Node_GetStoreItems::UK2Node_GetStoreItems()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UDreamAsyncProxy, CreatePDSAsyncGetStoreItems);
	ProxyFactoryClass = UDreamAsyncProxy::StaticClass();
	ProxyClass = UPDSAsync_GetStoreItems::StaticClass();
}

FText UK2Node_GetStoreItems::GetMenuCategory() const
{
	return FText::FromString(TEXT("PDS Async"));
}

FText UK2Node_GetStoreItems::GetTooltipText() const
{
	return FText::FromString(TEXT("获取商店得物品信息"));
}

FText UK2Node_GetStoreItems::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("PDS GetStoreItems"));
}