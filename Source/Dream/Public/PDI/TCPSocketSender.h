#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "NetworkMessage.h"
#include "Sockets.h"

class FTCPSocketSender : public FRunnable
{
	
public:

	FTCPSocketSender(FSocket* Socket) : bStopping(false)
	{
		SocketWrapper = new FSimpleAbstractSocket_FSocket(Socket);
		Event = FPlatformProcess::GetSynchEventFromPool();
		Thread = FRunnableThread::Create(this, TEXT("FTCPSocketSender"), 0, TPri_AboveNormal);
	}

	virtual ~FTCPSocketSender()
	{
		if (Thread)
		{
			Thread->Kill(true);
			delete Thread;
		}

		FPlatformProcess::ReturnSynchEventToPool(Event);
		Event = nullptr;
		
		delete SocketWrapper;
	}

	bool Send(TSharedRef<TArray<uint8>, ESPMode::ThreadSafe> Data)
	{
		if (!bStopping && SendQueue.Enqueue(Data))
		{
			Event->Trigger();
			
			return true;
		}

		return false;
	}

	bool Update()
	{
		while (!SendQueue.IsEmpty())
		{
			TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Data;
			SendQueue.Dequeue(Data);
			
#if !UE_BUILD_SHIPPING

			FString BytesStr;

			for (uint8 Byte : *Data)
			{
				BytesStr.Appendf(TEXT("%d, "), Byte);
			}

			UE_LOG(LogDream, Verbose, TEXT("Packet: %s"), *BytesStr);
			
#endif

			if (!SocketWrapper->Send(Data->GetData(), Data->Num()))
			{
				return false;
			}
		}
		return true;
	}

	virtual bool Init() override
	{
		return true;
	}

	virtual uint32 Run() override
	{
		while (!bStopping)
		{
			Event->Wait();
			
			if (!Update())
			{
				bStopping = true;

				return 0;
			}
		}

		return 0;
	}

	virtual void Stop() override
	{
		bStopping = true;
		Event->Trigger();
	}

	virtual void Exit() override { }

private:

	FEvent* Event;

	FRunnableThread* Thread;

	bool bStopping;

	TQueue<TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe>, EQueueMode::Mpsc> SendQueue;

	FSimpleAbstractSocket_FSocket* SocketWrapper;
};
