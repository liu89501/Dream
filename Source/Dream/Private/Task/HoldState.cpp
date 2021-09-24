// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppExpressionWithoutSideEffects
#include "HoldState.h"

UHoldState* UHoldState::CreateHoldState(UObject* WCO, float InHoldTime, const FHoldStateHandle& Handle)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WCO, EGetWorldErrorMode::LogAndReturnNull))
	{
		UHoldState* HoldState = NewObject<UHoldState>(WCO);
		HoldState->HoldTime = InHoldTime;
		HoldState->UpdateDelta = World->GetDeltaSeconds();
		HoldState->Handle_Task = Handle.Handle_Task;
		World->GetTimerManager().SetTimer(*Handle.Handle_Task, HoldState, &UHoldState::OnTaskTick, HoldState->UpdateDelta, true);
		return HoldState;
	}

	return nullptr;
}

void UHoldState::OnTaskTick()
{
	UpdateCount = FMath::Min(HoldTime, UpdateCount + UpdateDelta);

	OnUpdate.Broadcast(FMath::Min(UpdateCount / HoldTime, 1.f));
	if (FMath::IsNearlyEqual(UpdateCount, HoldTime))
	{
		if (Handle_Task.IsValid())
		{
			OnComplete.Broadcast(1.f);
			GetWorld()->GetTimerManager().ClearTimer(*Handle_Task);
		}
	}
}
