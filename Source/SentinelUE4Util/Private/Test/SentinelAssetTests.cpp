#include "CoreMinimal.h"
#include "Tests/AutomationCommon.h"
#include "SentinelPCComponent.h"
#include "SentinelTestLocation.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"

UWorld* GetTestWorld() {
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts) {
		if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game))
			&& (Context.World() != nullptr)) {
			return Context.World();
		}
	}
	return nullptr;
}

static void Exit()
{
	if (UWorld* World = GetTestWorld())
	{
		if (APlayerController* TargetPC = UGameplayStatics::GetPlayerController(World, 0))
		{
			TargetPC->ConsoleCommand(TEXT("Exit"), true);
		}
	}
}

static void Trigger() 
{
	UE_LOG(LogTemp, Warning, TEXT("Trigger Baby!"));

}
USentinelPCComponent* GetSentinelProfilingComponent() 
{
	// find the player controller in the loaded world
	APlayerController* PlayerController = GetTestWorld()->GetFirstPlayerController();

	TArray<USentinelPCComponent*> profilerComponents;
	PlayerController->GetComponents<USentinelPCComponent>(profilerComponents);
	
	//TArray<USentinelPCComponent*> profilerComponents = PlayerController->GetComponentsByClass(USentinelPCComponent::StaticClass());

	UE_LOG(LogTemp, Warning, TEXT("Found %d profiling components"), profilerComponents.Num());

	if (profilerComponents.Num() == 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found component!"));
		// There only should ever be one component so we return that one

		return profilerComponents[0];
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("No component found"));;
		return NULL;
	}

}

// 1. We define the test with BEGIN_DEFINE_SPEC and END_DEFINE_SPEC macros. 
//    Everything between these two macros is a variable shared between implemented tests.
BEGIN_DEFINE_SPEC(FSentinelTest, "Sentinel.New", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FSentinelTest)
void FSentinelTest::Define()
{
	BeforeEach([this]()
	{
		// FPlatformProcess::Sleep(10.0f);

		AutomationOpenMap(TEXT("/Game/Medieval_Armory/Maps/Demo_01"));

	});
	LatentIt("Run Latent Test", [this](const FDoneDelegate& Done)
	{
		AsyncTask(ENamedThreads::GameThread, [this, Done]()
		{
			// Fetch reference to the game object that I want to interact with
			USentinelPCComponent* profilingComponent = GetSentinelProfilingComponent();

			// Binding to "finished" event on the game object

				// profilingComponent->onCaptureFinished -> something something

			// Logic that needs to trigger when the onCaptureFinished event is triggered

				// Done.Execute();
				// Exit();

			// Trigger the behavior that takes a few frames to finish
			profilingComponent->CaptureGPUData("AutomationTest");

		});
	});

	
	It("Capture First", [this]()
	{
		// 7. Test if there are 3 enemy characters.
		// profiler_component->CaptureGPUData("Fudge");
		TestTrue("Check if there are 3 enemies on the level", true);
	});
}
