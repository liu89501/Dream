#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "PlayerDataStoreType.h"
#include "PDSAsync_SwitchWeapon.generated.h"


UCLASS()
class DREAM_API UPDSAsync_SwitchWeapon : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;

public:

	UFUNCTION()
	void OnLoadCompleted(const FString& ErrorMessage);

	void Init(int32 WeaponId, int32 EquipmentIndex);

private:

	FDelegateHandle Handle;
};
