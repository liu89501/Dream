// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DamageWidgetComponent.h"

UDamageWidgetComponent::UDamageWidgetComponent()
    : PendingDestroyTime(.4f)
{
    
}

void UDamageWidgetComponent::ActiveComponent(float Damage, bool bCritical, bool bIsHealthSteal)
{
    InitializeAttribute(Damage, bCritical, bIsHealthSteal);
    FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(Handle, this, &UDamageWidgetComponent::OnDestroyDamageWidgetComponent, PendingDestroyTime);
}

void UDamageWidgetComponent::OnDestroyDamageWidgetComponent()
{
    DestroyComponent();
}
