
#include "PDSAsync_Login.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceModule.h"

void UPDSAsync_Login::OnCompleted(const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}

	FPlayerDataInterfaceModule::Get()->OnLoginComplete.Remove(Handle);
}

void UPDSAsync_Login::Init()
{
	if (FPlayerDataInterface* PDS = FPlayerDataInterfaceModule::Get())
	{
		Handle = PDS->OnLoginComplete.AddUObject(this, &UPDSAsync_Login::OnCompleted);
		PDS->Login();
	}
}

