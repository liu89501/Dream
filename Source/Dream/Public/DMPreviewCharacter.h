// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DMPreviewActor.h"
#include "PlayerDataInterfaceType.h"
#include "DMPreviewCharacter.generated.h"

class AShootWeapon;

/**
 * 
 */
UCLASS()
class DREAM_API ADMPreviewCharacter : public ADMPreviewActor
{
	GENERATED_BODY()

public:

	ADMPreviewCharacter();

public:
	
	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void InitPreviewCharacter(int32 InUseWeaponSlot, const FPlayerInfo& PlayerInfo, class ADCharacterPlayer* OwnerPlayer);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void UpdatePreviewWeapon(TSubclassOf<AShootWeapon> WeaponClass, int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	USkeletalMeshComponent* GetCharacterMeshComponent() const;

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void DestroyComponentBySocketName(const FName& SocketName);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	void UpdateCharacterMesh(class UCharacterMesh* CharacterMesh);

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	int32 GetActiveWeaponAnimID() const;
	
	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	int32 GetActiveWeaponIndex() const;

	UFUNCTION(BlueprintCallable, Category=CharacterPreview)
	const FEquipmentAttributes& GetCharacterAttributes() const;

private:

	int32 UseWeaponSlot;

	UPROPERTY()
	USkeletalMeshComponent* PreviewCharacterMesh;

	UPROPERTY()
	TArray<AShootWeapon*> EquippedWeapons;

	UPROPERTY()
	FEquipmentAttributes CharacterAttributes;
};
