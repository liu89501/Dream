﻿#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "PlayerDataStoreType.h"
#include "PDSAsync_AddWeapon.generated.h"


UCLASS()
class DREAM_API UPDSAsync_AddWeapon : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;

public:

	UFUNCTION()
	void OnLoadCompleted(bool bSuccessful);

	void Init(const TArray<FPlayerWeaponAdd>& NewWeapons);

private:

	FDelegateHandle Handle;
};
