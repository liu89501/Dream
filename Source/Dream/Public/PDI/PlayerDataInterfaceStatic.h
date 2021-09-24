#pragma once

#include "CoreMinimal.h"

class FPlayerDataInterface;

class FPlayerDataInterfaceStatic
{

public:

	static void Startup();

	static void Shutdown();

	static FPlayerDataInterface* Get();
	
private:

	static FPlayerDataInterface* Singleton;
};