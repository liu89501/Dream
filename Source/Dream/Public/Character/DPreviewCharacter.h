// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DPreviewCharacter.generated.h"

UCLASS()
class DREAM_API ADPreviewCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADPreviewCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PreviewCharacter")
	FName ActiveSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PreviewCharacter")
	FName HolsterSocketName;

public:

	UFUNCTION(BlueprintCallable, Category = PreviewCharacter)
	int32 GetWeaponAnimId() const;
	
	void InitialCharacter(const struct FPlayerInfo& PlayerInfo, const FString& ErrorMessage);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	int32 CurrentWeaponAnimId;
};
