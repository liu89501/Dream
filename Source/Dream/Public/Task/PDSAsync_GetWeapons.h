﻿#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "PlayerDataStoreType.h"
#include "PDSAsync_GetWeapons.generated.h"

UCLASS()
class DREAM_API UPDSAsync_GetWeapons : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const TArray<FPlayerWeapon>&, Weapons);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;

public:

	UFUNCTION()
	void OnLoadCompleted(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage);

	void Init(EGetWeaponCondition Condition);

private:

	FDelegateHandle Handle;
};
