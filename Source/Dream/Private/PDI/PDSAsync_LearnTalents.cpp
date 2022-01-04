
#include "PDI/PDSAsync_LearnTalents.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_LearnTalents* UPDSAsync_LearnTalents::PDI_LearnTalents(UObject* WorldContextObject, int64 TalentIds)
{
	if (UPDSAsync_LearnTalents* PDSLT = NewObject<UPDSAsync_LearnTalents>(WorldContextObject))
	{
		PDSLT->T_TalentIds = TalentIds;
		return PDSLT;
	}
	return nullptr;
}

void UPDSAsync_LearnTalents::Activate()
{
	Handle = FPDIStatic::Get()->AddOnLearnTalents(FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_LearnTalents::OnCompleted));
	FPDIStatic::Get()->LearningTalents(T_TalentIds);
}

void UPDSAsync_LearnTalents::OnCompleted(bool bSuccess)
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
