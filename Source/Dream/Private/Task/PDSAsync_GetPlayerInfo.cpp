
#include "PDSAsync_GetPlayerInfo.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"


void UPDSAsync_GetPlayerInfo::OnLoadCompleted(const FPlayerInfo& PlayerInfo, const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(PlayerInfo, ErrorMessage);
	}
	else
	{
		OnFail.Broadcast(PlayerInfo, ErrorMessage);
	}

	FPlayerDataStoreModule::Get()->OnGetPlayerInfoComplete.Remove(Handle);
}

void UPDSAsync_GetPlayerInfo::Init()
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnGetPlayerInfoComplete.AddUObject(this, &UPDSAsync_GetPlayerInfo::OnLoadCompleted);
		PDS->GetPlayerInfo();
	}
}

