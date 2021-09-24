#pragma once

#include "CoreMinimal.h"
#include "LatentActions.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HoldState.generated.h"

USTRUCT(BlueprintType)
struct FHoldStateHandle
{
	GENERATED_BODY()

	FHoldStateHandle() : Handle_Task(nullptr)
	{
		Handle_Task = MakeShared<FTimerHandle>();
	}

	TSharedPtr<FTimerHandle> Handle_Task;
};

UCLASS()
class DREAM_API UHoldState : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, float, ProgressRate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate OnComplete;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate OnUpdate;

	public:

	UFUNCTION(BlueprintCallable, meta = (
		WorldContext = "WCO",
		BlueprintInternalUseOnly = "TRUE",
		DisplayName = "Hold State Ex",
		ScriptName = "Hold State Ex"), Category="DreamStatics|DreamTask")
    static class UHoldState* CreateHoldState(UObject* WCO, float InHoldTime, const FHoldStateHandle& Handle);

	void OnTaskTick();

	private:

	float HoldTime;
	float UpdateDelta;
	float UpdateCount;
	TSharedPtr<FTimerHandle> Handle_Task;
};
