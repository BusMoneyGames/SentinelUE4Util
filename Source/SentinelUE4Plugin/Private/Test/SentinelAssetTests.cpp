#include "CoreMinimal.h"
#include "Tests/AutomationCommon.h"
#include "SentinelProfiler.h"
#include "SentinelTestPosition.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "AssetRegistryModule.h"

void GetMapPackages() {
	// 1. Get the array of assets from Maps directory.
	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataArray;
	
	ARM.Get().GetAssetsByPath(TEXT("/Game/FirstPersonCPP/Maps"), AssetDataArray);
}

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


// Everything between these two macros is a variable shared between implemented tests.
BEGIN_DEFINE_SPEC(FSentinelAutomationPointsTest, "Sentinel.AutomationPoints", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
UWorld* World;
APlayerController* PController;
USentinelProfiler* profileComponent;
TArray<USentinelProfiler*> profilerComps;
END_DEFINE_SPEC(FSentinelAutomationPointsTest)


void FSentinelAutomationPointsTest::Define()
{
	Describe("Levels", [this]()
	{
		for (int32 Index = 0; Index < 5; Index++)
		{
			It(FString::Printf(TEXT("should resolve %d + %d = %d"), Index, 2, Index + 2), [this, Index]()
			{
				TestEqual(FString::Printf(TEXT("%d + %d = %d"), Index, 2, Index + 2), Index + 2, Index + 2);
			});
		}
	});

	LatentBeforeEach([this](const FDoneDelegate TestDone)
	{

		AutomationOpenMap(TEXT("/Game/Medieval_Armory/Maps/Demo_01"));

		World = GetTestWorld();
		PController = World->GetFirstPlayerController();

		TestNotNull("Check if World is properly created", World);
		TestNotNull("Check if Player Controller is found", PController);

		PController->GetComponents(profilerComps);			
		TestTrue("Check if profiling component is found on player controller", profilerComps.IsValidIndex(0));
		profileComponent = profilerComps[0];

		// Grace period to stream in textures and finish loading the level
		FPlatformProcess::Sleep(2.0f);
		TestDone.Execute();

	});

	LatentAfterEach([this](const FDoneDelegate TestDone) {
		
		Exit();
		TestDone.Execute();
	});

	LatentIt("Capture All the things", EAsyncExecution::ThreadPool, [this](const FDoneDelegate TestDone)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			profileComponent->CaptureGPUData("Latent Baby");
		
		});
		// Allow the test to run 
		// TODO hook this up to the event delegate in the profile component

		FPlatformProcess::Sleep(5.0f);

		TestDone.Execute();
	});

}
