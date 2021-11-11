
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
	Handle = FPDIStatic::Get()->AddOnLearnTalents(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_LearnTalents::OnCompleted));
	FPDIStatic::Get()->LearningTalents(T_TalentIdArray);
}

void UPDSAsync_LearnTalents::OnCompleted(bool bSuccess) const
{
	FPDIStatic::Get()->RemoveOnLearnTalents(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}
