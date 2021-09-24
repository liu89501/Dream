
#include "PDI/PDSAsync_Login.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_Login* UPDSAsync_Login::PDI_Login(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_Login* PDSL = NewObject<UPDSAsync_Login>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [PDSL]
        {
			FCommonCompleteNotify Delegate;
			Delegate.BindUObject(PDSL, &UPDSAsync_Login::OnCompleted);
			FPlayerDataInterfaceStatic::Get()->Login(Delegate);
			
        }, 0.001f, false);

		return PDSL;
	}

	return nullptr;
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

