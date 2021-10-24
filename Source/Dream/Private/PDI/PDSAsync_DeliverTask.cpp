
#include "PDI/PDSAsync_DeliverTask.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_DeliverTask* UPDSAsync_DeliverTask::PDI_DeliverTask(UObject* WorldContextObject, int64 TaskId)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_DeliverTask* PDSI = NewObject<UPDSAsync_DeliverTask>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [TaskId, PDSI]
        {
			FTaskRewardDelegate Delegate;
            Delegate.BindUObject(PDSI, &UPDSAsync_DeliverTask::OnCompleted);
            FPlayerDataInterfaceStatic::Get()->DeliverTask(TaskId, Delegate);
			
        }, 0.001f, false);
		
		return PDSI;
	}
	
	return nullptr;
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
