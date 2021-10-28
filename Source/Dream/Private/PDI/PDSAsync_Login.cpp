
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
	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &UPDSAsync_Login::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->Login(Delegate);
}

void UPDSAsync_Login::OnCompleted(const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

