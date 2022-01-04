// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IconInterface.h"
#include "GameFramework/Actor.h"
#include "DreamGuide.generated.h"

UCLASS()
class DREAM_API ADreamGuide : public AActor, public IIconInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADreamGuide();

	UPROPERTY(VisibleAnywhere)
	class UIconComponent* IconComponent;

	UPROPERTY(VisibleAnywhere)
	class UBillboardComponent* Billboard;

	/**
	 * 是否初始化完成是自动把自己注册到指引UI中
	 */
	UPROPERTY(EditInstanceOnly, Category=Guide)
	bool bAutoRegister;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category=Guide)
    void AddSelf();

	UFUNCTION(BlueprintCallable, Category=Guide)
    void RemoveSelf();
	
public:	

	virtual UIconComponent* GetIconComponent() const override;

};
