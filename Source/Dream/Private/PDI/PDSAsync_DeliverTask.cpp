
#include "PDI/PDSAsync_DeliverTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_DeliverTask* UPDSAsync_DeliverTask::PDI_DeliverTask(UObject* WorldContextObject, int64 TaskId)
{
	UPDSAsync_DeliverTask* PDSI = NewObject<UPDSAsync_DeliverTask>(WorldContextObject);
	PDSI->T_TaskId = TaskId;
	return PDSI;
}

void UPDSAsync_DeliverTask::OnCompleted(bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnDeliverTask(Handle);
	
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

void UPDSAsync_DeliverTask::Activate()
{
	Handle = FPDIStatic::Get()->AddOnDeliverTask(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_DeliverTask::OnCompleted));
	FPDIStatic::Get()->DeliverTask(T_TaskId);
}
