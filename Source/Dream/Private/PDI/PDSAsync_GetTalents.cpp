
#include "PDI/PDSAsync_GetTalents.h"
#include "DMProjectSettings.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetTalents* UPDSAsync_GetTalents::PDI_GetTalents(UObject* WorldContextObject, ETalentCategory Category)
{
	UPDSAsync_GetTalents* PDSGT = NewObject<UPDSAsync_GetTalents>(WorldContextObject);
	PDSGT->T_Category = Category;
	return PDSGT;
}

void UPDSAsync_GetTalents::Activate()
{
	Handle = GDataInterface->AddOnGetTalents(FOnGetTalents::FDelegate::CreateUObject(this, &UPDSAsync_GetTalents::OnCompleted));
	GDataInterface->GetTalents(T_Category);
}

void UPDSAsync_GetTalents::OnCompleted(int64 Talents, bool bSuccess)
{
	GDataInterface->RemoveOnGetTalents(Handle);

	TArray<FTalentInfo> TalentInfos;
	
	if (bSuccess)
	{
		GSProject->GetTalents(T_Category, Talents, TalentInfos);
		OnSuccess.Broadcast(TalentInfos);
	}
	else
	{
		OnFailure.Broadcast(TalentInfos);
	}
}
