
#include "PDI/PDSAsync_LearnTalents.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_LearnTalents* UPDSAsync_LearnTalents::PDI_LearnTalents(UObject* WorldContextObject, const TArray<int32>& TalentIdArray)
{
	if (UPDSAsync_LearnTalents* PDSLT = NewObject<UPDSAsync_LearnTalents>(WorldContextObject))
	{
		PDSLT->T_TalentIdArray = TalentIdArray;
		return PDSLT;
	}
	return nullptr;
}

void UPDSAsync_LearnTalents::Activate()
{
	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &UPDSAsync_LearnTalents::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->LearningTalents(T_TalentIdArray, Delegate);
}

void UPDSAsync_LearnTalents::OnCompleted(const FString& ErrorMessage) const
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
