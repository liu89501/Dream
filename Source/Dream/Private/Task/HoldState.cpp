// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppExpressionWithoutSideEffects
#include "HoldState.h"

UHoldState* UHoldState::CreateHoldState(UObject* WCO, float InHoldTime, const FHoldStateHandle& Handle)
{
	UHoldState* HoldState = NewObject<UHoldState>(WCO);
	HoldState->HoldTime = InHoldTime;
	HoldState->Handle_Task = Handle.Handle_Task;
	return HoldState;
}

void UHoldState::Activate()
{
	UpdateDelta = GetWorld()->GetDeltaSeconds();
	GetWorld()->GetTimerManager().SetTimer(*Handle_Task, this, &UHoldState::OnTaskTick, UpdateDelta, true);
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
