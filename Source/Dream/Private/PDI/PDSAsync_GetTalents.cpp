
#include "PDI/PDSAsync_GetTalents.h"


#include "DProjectSettings.h"
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
	Handle = FPDIStatic::Get()->AddOnGetTalents(FOnGetTalents::FDelegate::CreateUObject(this, &UPDSAsync_GetTalents::OnCompleted));
	FPDIStatic::Get()->GetTalents(T_Category);
}

void UPDSAsync_GetTalents::OnCompleted(int64 Talents, bool bSuccess)
{
	FPDIStatic::Get()->RemoveOnGetTalents(Handle);
	if (bSuccess)
	{
		OnSuccess.Broadcast(UDProjectSettings::GetProjectSettings()->GetTalents(T_Category, Talents));
	}
	else
	{
		TArray<FTalentInfo> Empty;
		OnFailure.Broadcast(Empty);
	}
}
