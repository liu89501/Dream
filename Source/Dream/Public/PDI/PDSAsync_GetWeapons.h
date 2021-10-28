#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_GetWeapons.generated.h"

UCLASS()
class UPDSAsync_GetWeapons : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetCompoletedDelegate, const TArray<FPlayerWeapon>&, Weapons);

public:

	UPROPERTY(BlueprintAssignable)
	FGetCompoletedDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FGetCompoletedDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
    static UPDSAsync_GetWeapons* PDI_GetWeapons(UObject* WorldContextObject, EGetEquipmentCondition Condition);

	void OnLoadCompleted(const TArray<FPlayerWeapon>& Weapons, const FString& ErrorMessage) const;

	virtual void Activate() override;

private:

	EGetEquipmentCondition T_Condition;
};
