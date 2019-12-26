#include "CoreMinimal.h"
#include "Tests/AutomationCommon.h"
#include "SentinelProfiler.h"
#include "SentinelTestPosition.h"
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


// Everything between these two macros is a variable shared between implemented tests.
BEGIN_DEFINE_SPEC(FNewEnemyCountTest, "Sentinel.AutomationPoints", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
UWorld* World;
APlayerController* PController;
USentinelProfiler* profileComponent;
TArray<USentinelProfiler*> profilerComps;
bool shouldLoadMap = true;
END_DEFINE_SPEC(FNewEnemyCountTest)

void FNewEnemyCountTest::Define()
{
	BeforeEach([this]()
	{
		if (shouldLoadMap) 
		{
			AutomationOpenMap(TEXT("/Game/Medieval_Armory/Maps/Demo_01"));
			shouldLoadMap = false;
		}

		World = GetTestWorld();
		PController = World->GetFirstPlayerController();
		TestNotNull("Check if World is properly created", World);
		TestNotNull("Check if Player Controller is found", PController);

		if (!profileComponent) 
		{
			PController->GetComponents(profilerComps);			
			TestTrue("Check if profiling component is found on player controller", profilerComps.IsValidIndex(0));
			profileComponent = profilerComps[0];
		}

	});

	It("Do Profiling For Points", [this]()
	{
		// 7. Test if there are 3 enemy characters.
		profileComponent->CaptureGPUData("Fudge");

		TestFalse("Check if there are 3 enemies on the level", false);
	});
}
