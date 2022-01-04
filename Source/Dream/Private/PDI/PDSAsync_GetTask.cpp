
#include "PDI/PDSAsync_GetTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetTask* UPDSAsync_GetTask::PDI_GetTask(UObject* WorldContextObject, int32 TaskGroupId, int32 Page, EGetTaskCondition Condition)
{
	UPDSAsync_GetTask* PDSI = NewObject<UPDSAsync_GetTask>(WorldContextObject);
	PDSI->T_Condition = Condition;
	PDSI->T_TaskGroupId = TaskGroupId;
	PDSI->T_Page = Page;
	return PDSI;
}

void UPDSAsync_GetTask::Activate()
{
	Handle = FPDIStatic::Get()->AddOnGetTasks(FOnGetTasks::FDelegate::CreateUObject(this, &UPDSAsync_GetTask::OnCompleted));
	FPDIStatic::Get()->GetTasks(FSearchTaskParam(T_Page, T_Condition, T_TaskGroupId));
}

void UPDSAsync_GetTask::OnCompleted(const FSearchTaskResult& Result, bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnGetTasks(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast(Result);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}
}
