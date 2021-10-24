
#include "PDI/PDSAsync_AcceptTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_AcceptTask* UPDSAsync_AcceptTask::PDI_AcceptTask(UObject* WorldContextObject, int64 InTaskId)
{
	UPDSAsync_AcceptTask* PDSI = NewObject<UPDSAsync_AcceptTask>(WorldContextObject);
	PDSI->TaskId = InTaskId;
	return PDSI;
}

void UPDSAsync_AcceptTask::OnCompleted(const FString& ErrorMessage) const
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

void UPDSAsync_AcceptTask::Activate()
{
	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &UPDSAsync_AcceptTask::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->AcceptTask(TaskId, Delegate);
}
