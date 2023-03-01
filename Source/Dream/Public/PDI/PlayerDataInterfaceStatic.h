#pragma once

#include "CoreMinimal.h"

extern class FPlayerDataInterface* GDataInterface;

class FPDIStatic
{

public:

	static void Initialize();

	static void Shutdown();
};