
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
	Handle = FPDIStatic::Get()->AddOnLogin(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_Login::OnCompleted));
	FPDIStatic::Get()->Login();
}

void UPDSAsync_Login::OnCompleted(bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnLogin(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

