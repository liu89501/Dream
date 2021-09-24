#include "PDI/PDSAsync_GetPlayerProperties.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetPlayerProperties* UPDSAsync_GetPlayerProperties::PDI_GetPlayerProperties(UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        UPDSAsync_GetPlayerProperties* PDSI = NewObject<UPDSAsync_GetPlayerProperties>(WorldContextObject);

        FTimerHandle Handle;
        World->GetTimerManager().SetTimer(Handle, [PDSI]
        {
            FGetPlayerPropertiesDelegate Delegate;
            Delegate.BindUObject(PDSI, &UPDSAsync_GetPlayerProperties::OnLoadCompleted);
            FPlayerDataInterfaceStatic::Get()->GetPlayerProperties(Delegate);
			
        }, 0.001f, false);
		
        return PDSI;
    }
    return nullptr;
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

