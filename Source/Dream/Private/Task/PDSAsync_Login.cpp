
#include "PDSAsync_Login.h"
#include "PlayerDataStore.h"
#include "PlayerDataStoreModule.h"

void UPDSAsync_Login::OnCompleted(const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}

	FPlayerDataStoreModule::Get()->OnLoginComplete.Remove(Handle);
}

void UPDSAsync_Login::Init()
{
	if (FPlayerDataStore* PDS = FPlayerDataStoreModule::Get())
	{
		Handle = PDS->OnLoginComplete.AddUObject(this, &UPDSAsync_Login::OnCompleted);
		PDS->Login();
	}
}

