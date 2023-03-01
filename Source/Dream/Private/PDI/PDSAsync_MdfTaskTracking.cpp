
#include "PDI/PDSAsync_MdfTaskTracking.h"
#include "PDI/PlayerDataInterface.h"
#include "PDI/PlayerDataInterfaceStatic.h"

UPDSAsync_MdfTaskTracking* UPDSAsync_MdfTaskTracking::PDI_ModifyTaskTracking(UObject* WorldContextObject, int64 InTaskId, bool bTrackingState)
{
	UPDSAsync_MdfTaskTracking* PDSI = NewObject<UPDSAsync_MdfTaskTracking>(WorldContextObject);
	PDSI->T_TaskId = InTaskId;
	PDSI->T_TrackingState = bTrackingState;
	return PDSI;
}

void UPDSAsync_MdfTaskTracking::OnCompleted(bool bSuccess)
{
	GDataInterface->RemoveOnModifyTaskTracking(Handle);
	
	if (bSuccess)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}

void UPDSAsync_MdfTaskTracking::Activate()
{
	Handle = GDataInterface->AddOnModifyTaskTracking(
		FOnCompleted::FDelegate::CreateUObject(this, &UPDSAsync_MdfTaskTracking::OnCompleted));
	
	GDataInterface->ModifyTrackingState(FModifyTrackingParam(T_TaskId, T_TrackingState));
}
