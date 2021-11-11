
#include "PDI/PDSAsync_DeliverTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_DeliverTask* UPDSAsync_DeliverTask::PDI_DeliverTask(UObject* WorldContextObject, int32 TaskId)
{
	UPDSAsync_DeliverTask* PDSI = NewObject<UPDSAsync_DeliverTask>(WorldContextObject);
	PDSI->T_TaskId = TaskId;
	return PDSI;
}

void UPDSAsync_DeliverTask::OnCompleted(UItemData* Rewards, bool bSuccess) const
{
	FPDIStatic::Get()->RemoveOnDeliverTask(Handle);
	
	if (bSuccess)
	{
		OnSuccess.Broadcast(Rewards);
	}
	else
	{
		OnFailure.Broadcast(Rewards);
	}
}

void UPDSAsync_DeliverTask::Activate()
{
	Handle = FPDIStatic::Get()->AddOnDeliverTask(FOnTaskReward::FDelegate::CreateUObject(this, &UPDSAsync_DeliverTask::OnCompleted));
	FPDIStatic::Get()->DeliverTask(T_TaskId);
}
