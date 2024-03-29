﻿#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_GetTalents.generated.h"


UCLASS()
class UPDSAsync_GetTalents : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const TArray<FTalentInfo>&, Talents);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_GetTalents* PDI_GetTalents(UObject* WorldContextObject, ETalentCategory Category);

	virtual void Activate() override;
	
	void OnCompleted(int64 Talents, bool bSuccess);

private:

	ETalentCategory T_Category;
	FDelegateHandle Handle;
};
