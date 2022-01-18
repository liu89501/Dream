// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DPropsType.h"
#include "GameFramework/Actor.h"
#include "GearBase.generated.h"

UENUM(BlueprintType)
enum class EGearType : uint8
{
	None,
    Head,
    Hand,
    Leg,
    Shoe
};

UCLASS()
class DREAM_API AGearBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGearBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* GearMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gear|Settings")
	FName AttachSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gear|Settings")
	FTransform AttachSocketTransform;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gear|Settings")
	EGearType Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gear|Settings")
	FEquipmentAttributes Attributes;


public:

	UPROPERTY(BlueprintReadOnly, Category = "Gear|Attribute")
	float CriticalRate;
	UPROPERTY(BlueprintReadOnly, Category = "Gear|Attribute")
	float CriticalDamage;
	UPROPERTY(BlueprintReadOnly, Category = "Gear|Attribute")
	float Defense;
	UPROPERTY(BlueprintReadOnly, Category = "Gear|Attribute")
	float MoveSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "Gear|Attribute")
	float DamageReduction;
	UPROPERTY(BlueprintReadOnly, Category = "Gear|Attribute")
	float Penetration;

public:

	virtual void Equipped(UPrimitiveComponent* TargetComponent);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
