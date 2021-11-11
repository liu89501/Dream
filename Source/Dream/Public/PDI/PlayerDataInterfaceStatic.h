#pragma once

#include "CoreMinimal.h"

class FPlayerDataInterface;
class UTaskDataAsset;

class FPDIStatic
{

public:

	static void Startup();

	static void Shutdown();

	static FPlayerDataInterface* Get();

	static bool IsLocalInterface();

	static UTaskDataAsset* GetTaskDataAsset();
	
private:

	static UTaskDataAsset* TaskDataAsset;
	
	static FPlayerDataInterface* Singleton;
};