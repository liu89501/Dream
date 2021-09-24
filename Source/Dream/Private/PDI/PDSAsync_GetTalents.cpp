
#include "PDI/PDSAsync_GetTalents.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_GetTalents* UPDSAsync_GetTalents::PDI_GetTalents(UObject* WorldContextObject, TEnumAsByte<EPDTalentCategory::Type> Category)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UPDSAsync_GetTalents* PDSGT = NewObject<UPDSAsync_GetTalents>(WorldContextObject);

		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [Category, PDSGT]
        {
			FGetTalentsComplete Delegate;
            Delegate.BindUObject(PDSGT, &UPDSAsync_GetTalents::OnCompleted);
            FPlayerDataInterfaceStatic::Get()->GetTalents(Category, Delegate);
			
        }, 0.001f, false);
		
		return PDSGT;
	}
	
	return nullptr;
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
