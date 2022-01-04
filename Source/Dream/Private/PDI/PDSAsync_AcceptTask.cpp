
#include "PDI/PDSAsync_AcceptTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_AcceptTask* UPDSAsync_AcceptTask::PDI_AcceptTask(UObject* WorldContextObject, int64 InTaskId)
{
	UPDSAsync_AcceptTask* PDSI = NewObject<UPDSAsync_AcceptTask>(WorldContextObject);
	PDSI->TaskId = InTaskId;
	return PDSI;
}

void UPDSAsync_AcceptTask::OnCompleted(bool bSuccess) 
{
	FPDIStatic::Get()->RemoveOnAcceptTask(Handle);
	
	if (bSuccess)
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
	Handle = FPDIStatic::Get()->AddOnAcceptTask(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_AcceptTask::OnCompleted));
	FPDIStatic::Get()->AcceptTask(FAcceptTaskParam(TaskId));
}
