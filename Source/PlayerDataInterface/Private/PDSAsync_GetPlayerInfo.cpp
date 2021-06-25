
#include "PDSAsync_GetPlayerInfo.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"


void UPDSAsync_GetPlayerInfo::OnLoadCompleted(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(PlayerInfo, ErrorMessage);
	}
	else
	{
		OnFail.Broadcast(PlayerInfo, ErrorMessage);
	}

	FPlayerDataInterfaceModule::Get()->OnGetPlayerInfoComplete.Remove(Handle);
}

void UPDSAsync_GetPlayerInfo::Init()
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnGetPlayerInfoComplete.AddUObject(this, &UPDSAsync_GetPlayerInfo::OnLoadCompleted);
		PDS->GetPlayerInfo();
	}
}

