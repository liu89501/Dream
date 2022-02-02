#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerDataInterfaceType.h"
#include "PDSAsync_GetPlayerInfo.generated.h"

UCLASS()
class UPDSAsync_GetPlayerInfo : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskCompoleteDelegate, const FPlayerInfo&, PlayerInfo);

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FAsyncTaskCompoleteDelegate OnFailure;

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"), Category=PDI)
	static UPDSAsync_GetPlayerInfo* PDI_GetPlayerInformation(UObject* WorldContextObject,
																bool bWeapon,
																bool bWeaponEquippedOnly,
																bool bModule,
																bool bModuleEquippedOnly,
																bool bMaterials,
																bool bSkin);

	void OnLoadCompleted(const FPlayerInfo& PlayerInfo, bool bSuccess);

	virtual void Activate() override;

private:

	int32 T_Condition;
	FDelegateHandle Handle;
};
