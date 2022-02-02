// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IconComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAM_API UIconComponent : public UActorComponent
{
	GENERATED_BODY()

	UIconComponent();

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Icon")
	bool bFixedDirection;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Icon")
	FSlateBrush Icon;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
