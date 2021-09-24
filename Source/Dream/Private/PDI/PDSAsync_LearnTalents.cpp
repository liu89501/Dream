
#include "PDI/PDSAsync_LearnTalents.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_LearnTalents* UPDSAsync_LearnTalents::PDI_LearnTalents(UObject* WorldContextObject, const TArray<int32>& TalentIdArray)
{
	if (UPDSAsync_LearnTalents* PDSLT = NewObject<UPDSAsync_LearnTalents>(WorldContextObject))
	{
		FCommonCompleteNotify Delegate;
		Delegate.BindUObject(PDSLT, &UPDSAsync_LearnTalents::OnCompleted);
		FPlayerDataInterfaceStatic::Get()->LearningTalents(TalentIdArray, Delegate);
		return PDSLT;
	}
	return nullptr;
}

void UPDSAsync_LearnTalents::OnCompleted(const FString& ErrorMessage) const
{
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, ErrorMessage]
    {
        if (ErrorMessage.IsEmpty())
        {
            OnSuccess.Broadcast();
        }
        else
        {
            OnFailure.Broadcast();
        }
		
    }, 0.001f, false);
}
