
#include "PDI/PDSAsync_GetTalents.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetTalents* UPDSAsync_GetTalents::PDI_GetTalents(UObject* WorldContextObject, TEnumAsByte<EPDTalentCategory::Type> Category)
{
	UPDSAsync_GetTalents* PDSGT = NewObject<UPDSAsync_GetTalents>(WorldContextObject);
	PDSGT->T_Category = Category;
	return PDSGT;
}

void UPDSAsync_GetTalents::Activate()
{
	FGetTalentsComplete Delegate;
	Delegate.BindUObject(this, &UPDSAsync_GetTalents::OnCompleted);
	FPlayerDataInterfaceStatic::Get()->GetTalents(T_Category, Delegate);
}

void UPDSAsync_GetTalents::OnCompleted(const TArray<FTalentInfo>& Talents, const FString& ErrorMessage) const
{
	if (ErrorMessage.IsEmpty())
	{
		OnSuccess.Broadcast(Talents);
	}
	else
	{
		TArray<FTalentInfo> Empty;
		OnFailure.Broadcast(Empty);
	}
}
