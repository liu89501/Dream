
#include "PDI/PDSAsync_DeliverTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_DeliverTask* UPDSAsync_DeliverTask::PDI_DeliverTask(UObject* WorldContextObject, int64 TaskId)
{
	UPDSAsync_DeliverTask* PDSI = NewObject<UPDSAsync_DeliverTask>(WorldContextObject);
	PDSI->T_TaskId = TaskId;
	return PDSI;
}

void UPDSAsync_DeliverTask::OnCompleted(UItemData* Rewards, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
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
	FTaskRewardDelegate Delegate;
	Delegate.BindUObject(this, &UPDSAsync_DeliverTask::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->DeliverTask(T_TaskId, Delegate);
}
