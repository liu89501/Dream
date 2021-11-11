
#include "PDI/PDSAsync_GetTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetTask* UPDSAsync_GetTask::PDI_GetTask(UObject* WorldContextObject, int32 TaskGroupId, EGetTaskCondition Condition)
{
	UPDSAsync_GetTask* PDSI = NewObject<UPDSAsync_GetTask>(WorldContextObject);
	PDSI->T_Condition = Condition;
	PDSI->T_TaskGroupId = TaskGroupId;
	return PDSI;
}

void UPDSAsync_GetTask::Activate()
{
	Handle = FPDIStatic::Get()->AddOnGetTasks(FOnGetTasks::FDelegate::CreateUObject(this, &UPDSAsync_GetTask::OnCompleted));
	FPDIStatic::Get()->GetTasks(FSearchTaskParam(T_Condition, T_TaskGroupId));
}

void UPDSAsync_GetTask::OnCompleted(const TArray<FTaskInformation>& Tasks, bool bSuccess) const
{
	FPDIStatic::Get()->RemoveOnGetTasks(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast(Tasks);
	}
	else
	{
		OnFailure.Broadcast(Tasks);
	}
}
