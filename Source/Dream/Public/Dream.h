// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define NetDebugMsg(...) UE_LOG(LogTemp, Error, TEXT("%s: %s"), *UEnum::GetValueAsString(GetLocalRole()), *FString::Printf(##__VA_ARGS__));
#define DebugMsg(...) UE_LOG(LogTemp, Error, TEXT("%s"), *FString::Printf(##__VA_ARGS__));

#define DreamDrawDebugLine(Start, End, Color) DrawDebugLine(GetWorld(), Start, End, Color, false, 3.f, 0, 1.f)
#define DreamDrawDebugSphere(Start, Radius, Color) DrawDebugSphere(GetWorld(), Start, Radius, 12, Color, false, 3.f, 0, 1.f)

#define Start_Delay(Handle, Func, Time) GetWorldTimerManager().SetTimer(Handle, this, Func, Time, false)
#define Start_Delay_L(Handle, Func, Time) GetWorldTimerManager().SetTimer(Handle, Func, Time, false)
#define Clear_Delay(Handle) if (Handle.IsValid()) {GetWorldTimerManager().ClearTimer(Handle);}
