#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_EquipModule.generated.h"

class ADCharacterPlayer;

UCLASS()
class UPDSAsync_EquipModule : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncTaskCompoleteDelegate);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate	OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "TRUE"), Category="PDI")
    static UPDSAsync_EquipModule* PDI_EquipModule(ADCharacterPlayer* Character, int64 ModuleId, EModuleCategory Category);

	virtual void Activate() override;

	UFUNCTION()
	void OnCompleted(const FPlayerModule& Module, bool bSuccess);

private:

	UPROPERTY()
	ADCharacterPlayer* T_Character;
	
	int64 T_ModuleId;
	EModuleCategory T_Category;
	FDelegateHandle Handle;
};
