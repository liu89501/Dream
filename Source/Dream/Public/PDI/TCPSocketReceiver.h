// ReSharper disable All
#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "ArrayReader.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "NetworkMessage.h"
#include "Sockets.h"

typedef TSharedRef<TArray<uint8>, ESPMode::ThreadSafe> FReceivePacket;

DECLARE_DELEGATE(FOnReceiverShutdown)
DECLARE_DELEGATE_OneParam(FReceiveDataTrigger, FReceivePacket)

class FTCPSocketReceiver : public FRunnable
{
	
public:

	FTCPSocketReceiver(FSocket* Socket):
		bStopping(false),
		bReadData(false),
		ReceiveDataLength(0),
		Thread(nullptr)
	{
		SocketWrapper = new FSimpleAbstractSocket_FSocket(Socket);
	}

	~FTCPSocketReceiver()
	{
		delete SocketWrapper;
		
		if (Thread)
		{
			Thread->Kill();
			delete Thread;
		}
	}

	FReceiveDataTrigger& OnReceiveDelegate()
	{
		return ReceiveTrigger;
	}

	FOnReceiverShutdown& OnShutdownDelegate()
	{
		return OnShutdown;
	}

	void Start()
	{
		Thread = FRunnableThread::Create(this, TEXT("FTCPSocketReceiver"), 0, TPri_AboveNormal);
	}

	virtual bool Init() override
	{
		return true;
	}

	virtual uint32 Run() override
	{
		while (!bStopping)
		{
			if (bReadData)
			{
				FReceivePacket Packet = MakeShared<TArray<uint8>, ESPMode::ThreadSafe>();
				Packet->SetNumUninitialized(ReceiveDataLength);
				
				if (!SocketWrapper->Receive(Packet->GetData(), ReceiveDataLength))
				{
					break;
				}

				if (ReceiveTrigger.IsBound())
				{
					
#if WITH_EDITOR

					/*FString BytesStr;

					for (uint8& Byte : *Packet)
					{
						BytesStr.Appendf(TEXT("%d, "), Byte);
					}

					UE_LOG(LogDream, Verbose, TEXT("Receive Packet: %s"), *BytesStr);*/
			
#endif
					
					ReceiveTrigger.Execute(Packet);
				}
			}
			else
			{
				uint8 Length[4];
				if (!SocketWrapper->Receive(Length, 4))
				{
					break;
				}

				ReceiveDataLength = Length[0] | Length[1] << 8 | Length[2] << 16 | Length[3] << 24;
			}

			bReadData = !bReadData;
		}

		if (!bStopping)
		{
			UE_LOG(LogDream, Warning, TEXT("FTCPSocketReceiver: 连接已断开"));
			OnShutdown.ExecuteIfBound();
		}

		return 0;
	}

	virtual void Stop() override
	{
		bStopping = true;
	}

	virtual void Exit() override { }

private:

	bool bStopping;

	bool bReadData;

	int32 ReceiveDataLength;

	FRunnableThread* Thread;
	
	FSimpleAbstractSocket_FSocket* SocketWrapper;

	FReceiveDataTrigger ReceiveTrigger;

	FOnReceiverShutdown OnShutdown;
	
};
