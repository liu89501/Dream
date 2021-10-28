#include "PDI/PDSAsync_GetPlayerProperties.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetPlayerProperties* UPDSAsync_GetPlayerProperties::PDI_GetPlayerProperties(UObject* WorldContextObject)
{
    return NewObject<UPDSAsync_GetPlayerProperties>(WorldContextObject);
}

void UPDSAsync_GetPlayerProperties::OnLoadCompleted(const FPlayerProperties& Properties, const FString& ErrorMessage) const
{
    if (ErrorMessage.IsEmpty())
    {
        OnSuccess.Broadcast(Properties);
    }
    else
    {
        OnFailure.Broadcast(Properties);
    }
}

void UPDSAsync_GetPlayerProperties::Activate()
{
    FGetPlayerPropertiesDelegate Delegate;
    Delegate.BindUObject(this, &UPDSAsync_GetPlayerProperties::OnLoadCompleted);
    FPlayerDataInterfaceStatic::Get()->GetPlayerProperties(Delegate);
}

