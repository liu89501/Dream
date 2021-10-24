// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthWidgetComponent.h"
#include "HealthUIInterface.h"

UHealthWidgetComponent::UHealthWidgetComponent()
{
    
}

void UHealthWidgetComponent::UpdateStatus(float HealthPercentage) const
{
    if (UUserWidget* WidgetObject = GetUserWidgetObject())
    {
        IHealthUIInterface::Execute_UpdateUI(WidgetObject, HealthPercentage);
    }
}
