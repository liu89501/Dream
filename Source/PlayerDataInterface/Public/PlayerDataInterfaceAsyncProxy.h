#pragma once

#include "CoreMinimal.h"
#include "PlayerDataInterfaceType.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlayerDataInterfaceAsyncProxy.generated.h"

UCLASS()
class PLAYERDATAINTERFACE_API UPlayerDataInterfaceAsyncProxy : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_GetWeapons* CreatePDSAsyncGetWeapons(UObject* WorldContextObject, EGetWeaponCondition Condition);
    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_SwitchWeapon* CreatePDSAsyncSwitchWeapon(UObject* WorldContextObject, int32 WeaponId, int32 EquipmentIndex);
    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_RunServer* CreatePDSAsyncRunServer(UObject* WorldContextObject, const FRunServerParameter& Parameter);
    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_AddWeapon* CreatePDSAsyncAddWeapon(UObject* WorldContextObject, TArray<FPlayerWeaponAdd> NewWeapons);
    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_PayItem* CreatePDSAsyncPayItem(UObject* WorldContextObject, int64 ItemId);
    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_GetStoreItems* CreatePDSAsyncGetStoreItems(UObject* WorldContextObject, int32 StoreId);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_GetPlayerInfo* CreatePDSAsyncGetPlayerInfo(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "TRUE"))
    static class UPDSAsync_Login* CreatePDSAsyncLogin(UObject* WorldContextObject);
};
