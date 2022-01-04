#pragma once

#include "CoreMinimal.h"

class FPlayerDataInterface;

class FPDIStatic
{

public:

	static void Initialize();

	static void Shutdown();

	static FPlayerDataInterface* Get();

	static bool IsLocalInterface();
	
	
private:

	static FPlayerDataInterface* Singleton;
};