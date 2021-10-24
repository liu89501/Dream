// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BroadcastReceiverComponent.h"


// Sets default values for this component's properties
UBroadcastReceiverComponent::UBroadcastReceiverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UBroadcastReceiverComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() > NM_DedicatedServer)
	{
		// 非 服务器或单机 不执行下面的逻辑
		return;
	}

	for (const FTargetDelegate& Description : Descriptions)
	{
		if (Description.Target == nullptr)
		{
			continue;
		}
		
		for (TFieldIterator<FMulticastDelegateProperty> It(Description.Target->GetClass()); It; ++It)
		{
			if (Description.DelegateName == It->GetFName())
			{
				FScriptDelegate Delegate;
				Delegate.BindUFunction(GetOwner(), Description.TriggerFunctionName);
				It->AddDelegate(MoveTemp(Delegate), Description.Target);
				break;
			}
		}
	}
}


// Called every frame
void UBroadcastReceiverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

