
#include "PDI/PDSAsync_Login.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_Login* UPDSAsync_Login::PDI_Login(UObject* WorldContextObject)
{
	UPDSAsync_Login* PDSL = NewObject<UPDSAsync_Login>(WorldContextObject);
	return PDSL;
}

void UPDSAsync_Login::Activate()
{
	Handle = GDataInterface->AddOnLogin(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_Login::OnCompleted));
	GDataInterface->Login();
}

void UPDSAsync_Login::OnCompleted(bool bSuccess)
{
	GDataInterface->RemoveOnLogin(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

