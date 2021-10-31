// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameInstance.h"

#include "DGameUserSettings.h"
#include "Engine.h"
#include "DreamType.h"
#include "IPv4Endpoint.h"
#include "MoviePlayer.h"
#include "SocketSubsystem.h"
#include "TcpSocketBuilder.h"
#include "PDI/PlayerDataInterfaceStatic.h"
#include "UserWidget.h"

UDreamGameInstance::UDreamGameInstance()
{

}

FSurfaceImpactEffect EmptySurfaceImpactEffect = FSurfaceImpactEffect();

const FSurfaceImpactEffect& UDreamGameInstance::GetSurfaceImpactEffect(EPhysicalSurface SurfaceType)
{
	const FSurfaceImpactEffect* EffectPtr = SurfaceImpactEffects.Find(SurfaceType);
	return EffectPtr ? *EffectPtr : EmptySurfaceImpactEffect;
}

void UDreamGameInstance::Init()
{
	Super::Init();

	FPlayerDataInterfaceStatic::Startup();

	if (!IsRunningDedicatedServer())
	{
		LoadingScreenWidget = CreateWidget(this, LoadingScreenWidgetClass);
		FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UDreamGameInstance::OnPreLoadMap);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UDreamGameInstance::OnPostLoadMap);
	}
}

void UDreamGameInstance::Shutdown()
{
	Super::Shutdown();

	FPlayerDataInterfaceStatic::Shutdown();
}

void UDreamGameInstance::OnPreLoadMap(const FString& MapName)
{
	/*if (MapName == TEXT("/Game/Maps/MainMenuUI"))
	{
		return;
	}*/

	FLoadingScreenAttributes Attr;
	Attr.WidgetLoadingScreen = LoadingScreenWidget->TakeWidget();
	GetMoviePlayer()->SetupLoadingScreen(Attr);
}

void UDreamGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	GetMoviePlayer()->StopMovie();
}

#if WITH_EDITORONLY_DATA

void UDreamGameInstance::TestSocketSend()
{
	FSocket* TestSocket = FTcpSocketBuilder(TEXT("TestSocket"))
		.AsBlocking()
		.AsReusable()
        .WithReceiveBufferSize(2 * 1024 * 1024)
        .WithSendBufferSize(1024 * 1024);

	FIPv4Endpoint Endpoint(FIPv4Address::InternalLoopback, 8888);
	TSharedRef<FInternetAddr> AddrIpv4 = Endpoint.ToInternetAddrIPV4();
	TestSocket->Bind(*AddrIpv4);
	TestSocket->Connect(*AddrIpv4);
	
	TArray<uint8> Data;
	FMemoryWriter Writer(Data);

	uint32 Value = 666;
	Writer << Value;

	int32 Sent;	
	TestSocket->Send(Data.GetData(), Data.Num(), Sent);
	

	UE_LOG(LogDream, Warning, TEXT("BytesSent: %d"), Sent);

	TestSocket->Close();
}

#endif
