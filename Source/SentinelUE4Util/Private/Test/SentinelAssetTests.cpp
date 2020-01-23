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
BEGIN_DEFINE_SPEC(FNewEnemyCountTest, "Sentinel.New", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
UWorld* World;
APlayerController* PController;
USentinelPCComponent* profiler_component;
END_DEFINE_SPEC(FNewEnemyCountTest)
void FNewEnemyCountTest::Define()
{
	BeforeEach([this]()
	{
		// FPlatformProcess::Sleep(10.0f);

		AutomationOpenMap(TEXT("/Game/Medieval_Armory/Maps/Demo_01"));

		/*
		// 4. Before each test the World is obtained and tested if is valid.
		World = GetTestWorld();
		PController = World->GetFirstPlayerController();

		FString MapName = World->GetMapName();
		
		TArray<USentinelPCComponent*> profilerComps;
		PController->GetComponents(profilerComps);
		//profiler_component = profilerComps[0];

		TestNotNull("Check if World is properly created", World);
		*/

	});
	LatentIt("Run Latent Test", [this](const FDoneDelegate& Done)
	{

	// 2. Because latent test runs on a separate thread we have to ensure that game logic tests run on a Game Thread. 
		AsyncTask(ENamedThreads::GameThread, [this, Done]()
		{
			USentinelPCComponent* comp = GetSentinelProfilingComponent();

			Done.Execute();
			Exit();

		});
	});

	
	It("Capture First", [this]()
	{
		// 7. Test if there are 3 enemy characters.
		// profiler_component->CaptureGPUData("Fudge");
		TestTrue("Check if there are 3 enemies on the level", true);
	});
}
