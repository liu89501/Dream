// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DItemPreview.h"
#include "DreamType.h"
#include "PlayerDataInterfaceType.h"
#include "ShootWeapon.h"


#include "DCharacterPreview.generated.h"

/**
 * 
 */
UCLASS()
class DREAM_API ADCharacterPreview : public ADItemPreview
{
	GENERATED_BODY()

public:

	ADCharacterPreview();
	
	
	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void InitPreviewCharacterFromPlayerInfo(int32 InUseWeaponSlot, const FPlayerInfo& PlayerInfo);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void UpdatePreviewWeapon(TSubclassOf<AShootWeapon> WeaponClass, int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	USkeletalMeshComponent* GetCharacterMeshComponent() const;

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void DestroyComponentBySocketName(const FName& SocketName);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void UpdateCharacterMesh(UCharacterMesh* CharacterMesh);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	int32 GetActiveWeaponAnimID() const;
	
	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	int32 GetActiveWeaponIndex() const;

private:

	int32 UseWeaponSlot;

	UPROPERTY()
	USkeletalMeshComponent* PreviewCharacterMesh;

	UPROPERTY()
	TArray<AShootWeapon*> EquippedWeapons;
};
