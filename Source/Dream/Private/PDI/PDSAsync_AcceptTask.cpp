
#include "PDI/PDSAsync_AcceptTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_AcceptTask* UPDSAsync_AcceptTask::PDI_AcceptTask(UObject* WorldContextObject, int32 InTaskId)
{
	UPDSAsync_AcceptTask* PDSI = NewObject<UPDSAsync_AcceptTask>(WorldContextObject);
	PDSI->TaskId = InTaskId;
	return PDSI;
}

void UPDSAsync_AcceptTask::OnCompleted(bool bSuccess) const
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

	FTaskInformation Information;
	
	UTaskDataAsset* TaskDataAsset = FPDIStatic::GetTaskDataAsset();
	if (TaskDataAsset->GetInformationByTaskId(TaskId, Information))
	{
		FPDIStatic::Get()->AcceptTask(FAcceptTaskParam(TaskId, Information.TaskGroupId, Information.CompleteCondition->TargetValue));
	}
	else
	{
		OnFailure.Broadcast();
	}
}
