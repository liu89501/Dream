#include "PlayerDataInterfaceAsyncProxy.h"
#include "PDSAsync_AddWeapon.h"
#include "PDSAsync_GetPlayerInfo.h"
#include "PDSAsync_GetStoreItems.h"
#include "PDSAsync_GetWeapons.h"
#include "PDSAsync_Login.h"
#include "PDSAsync_PayItem.h"
#include "PDSAsync_RunServer.h"
#include "PDSAsync_SwitchWeapon.h"

UPDSAsync_GetWeapons* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncGetWeapons(UObject* WorldContextObject, EGetWeaponCondition Condition)
{
    if (UPDSAsync_GetWeapons* PDSLP = NewObject<UPDSAsync_GetWeapons>(WorldContextObject))
    {
        PDSLP->Init(Condition);
        return PDSLP;
    }
    return nullptr;
}

UPDSAsync_SwitchWeapon* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncSwitchWeapon(UObject* WorldContextObject, int32 WeaponId, int32 EquipmentIndex)
{
    if (UPDSAsync_SwitchWeapon* PDSSW = NewObject<UPDSAsync_SwitchWeapon>(WorldContextObject))
    {
        PDSSW->Init(WeaponId, EquipmentIndex);
        return PDSSW;
    }
    return nullptr;
}

UPDSAsync_RunServer* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncRunServer(UObject* WorldContextObject, const FRunServerParameter& Parameter)
{
    if (UPDSAsync_RunServer* PDSRS = NewObject<UPDSAsync_RunServer>(WorldContextObject))
    {
        PDSRS->Init(Parameter);
        return PDSRS;
    }
    return nullptr;
}

UPDSAsync_AddWeapon* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncAddWeapon(UObject* WorldContextObject, TArray<FPlayerWeaponAdd> NewWeapons)
{
    if (UPDSAsync_AddWeapon* PDSAW = NewObject<UPDSAsync_AddWeapon>(WorldContextObject))
    {
        PDSAW->Init(NewWeapons);
        return PDSAW;
    }
    return nullptr;
}

UPDSAsync_PayItem* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncPayItem(UObject* WorldContextObject, int64 ItemId)
{
    if (UPDSAsync_PayItem* PDSPI = NewObject<UPDSAsync_PayItem>(WorldContextObject))
    {
        PDSPI->Init(ItemId);
        return PDSPI;
    }
    return nullptr;
}

UPDSAsync_GetStoreItems* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncGetStoreItems(UObject* WorldContextObject, int32 StoreId)
{
    if (UPDSAsync_GetStoreItems* PDSGSI = NewObject<UPDSAsync_GetStoreItems>(WorldContextObject))
    {
        PDSGSI->Init(StoreId);
        return PDSGSI;
    }
    return nullptr;
}

UPDSAsync_GetPlayerInfo* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncGetPlayerInfo(UObject* WorldContextObject)
{
    if (UPDSAsync_GetPlayerInfo* PDSGPI = NewObject<UPDSAsync_GetPlayerInfo>(WorldContextObject))
    {
        PDSGPI->Init();
        return PDSGPI;
    }
    return nullptr;
}


UPDSAsync_Login* UPlayerDataInterfaceAsyncProxy::CreatePDSAsyncLogin(UObject* WorldContextObject)
{
    if (UPDSAsync_Login* PDSLOGIN = NewObject<UPDSAsync_Login>(WorldContextObject))
    {
        PDSLOGIN->Init();
        return PDSLOGIN;
    }
    return nullptr;
}
