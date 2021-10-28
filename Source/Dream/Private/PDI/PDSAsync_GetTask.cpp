
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
	FGetTasksDelegate Delegate;
	Delegate.BindUObject(this, &UPDSAsync_GetTask::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->GetTasks(T_Condition, Delegate);
}

void UPDSAsync_GetTask::OnCompleted(const TArray<FTaskInformation>& Tasks, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Tasks);
	}
	else
	{
		OnFailure.Broadcast(Tasks);
	}
}
