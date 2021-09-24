
#include "PDI/PDSAsync_GetPlayerInfo.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"


UPDSAsync_GetPlayerInfo* UPDSAsync_GetPlayerInfo::PDI_GetPlayerInformation(UObject* WorldContextObject, EGetEquipmentCondition Condition)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_GetPlayerInfo* PDSGP = NewObject<UPDSAsync_GetPlayerInfo>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [PDSGP, Condition]
        {
			FGetPlayerInfoComplete Delegate;
            Delegate.BindUObject(PDSGP, &UPDSAsync_GetPlayerInfo::OnLoadCompleted);
            FPlayerDataInterfaceStatic::Get()->GetPlayerInfo(Condition, Delegate);
			
        }, 0.001f, false);

		return PDSGP;
	}

	return nullptr;
}

void UPDSAsync_GetPlayerInfo::OnLoadCompleted(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(PlayerInfo, ErrorMessage);
	}
	else
	{
		OnFailure.Broadcast(PlayerInfo, ErrorMessage);
	}
}

