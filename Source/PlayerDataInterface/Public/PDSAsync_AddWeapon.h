#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_AddWeapon.generated.h"


UCLASS()
class PLAYERDATAINTERFACE_API UPDSAsync_AddWeapon : public UObject
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFail;

public:

	UFUNCTION()
	void OnLoadCompleted(bool bSuccessful) const;

	void Init(const TArray<FPlayerWeaponAdd>& NewWeapons);

private:

	FDelegateHandle Handle;
};
