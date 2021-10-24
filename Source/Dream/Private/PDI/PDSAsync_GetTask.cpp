
#include "PDI/PDSAsync_GetTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetTask* UPDSAsync_GetTask::PDI_GetTask(UObject* WorldContextObject, int32 TaskGroupId, EGetTaskCondition Condition)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_GetTask* PDSI = NewObject<UPDSAsync_GetTask>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [Condition, PDSI]
        {
			FGetTasksDelegate Delegate;
            Delegate.BindUObject(PDSI, &UPDSAsync_GetTask::OnCompleted);
            FPlayerDataInterfaceStatic::Get()->GetTasks(Condition, Delegate);
			
        }, 0.001f, false);
		
		return PDSI;
	}
	
	return nullptr;
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
