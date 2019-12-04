#include "CoreMinimal.h"
#include "Tests/AutomationCommon.h"
#include "SentinelProfiler.h"
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

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSentinelProfilingCommand, FString, Command);
bool FSentinelProfilingCommand::Update()
{
	UWorld* world = GetTestWorld();

	APlayerController* PController = world->GetFirstPlayerController();

	TArray<USentinelProfiler*> profilerComps;
	PController->GetComponents(profilerComps);

	USentinelProfiler* profiler_component = profilerComps[0];

	if (!profiler_component->isProfiling)
	{
		profiler_component->CaptureGPUData("Code_Test");
	}

	return profiler_component->isProfiling;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSentinelAutomationCommandlet, "Sentinel.Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSentinelAutomationCommandlet::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/Map_Grassland01"));
	
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(10.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FSentinelProfilingCommand(""));
	
	UWorld* world = GetTestWorld();

	APlayerController* PController = world->GetFirstPlayerController();
	//ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("exit")));

	// FGenericPlatformMisc::RequestExit(false);
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(5));

	ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);

    return true;
}


// 1. We define the test with BEGIN_DEFINE_SPEC and END_DEFINE_SPEC macros. 
//    Everything between these two macros is a variable shared between implemented tests.
BEGIN_DEFINE_SPEC(FNewEnemyCountTest, "Sentinel.New", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
UWorld* World;
APlayerController* PController;
USentinelProfiler* profiler_component;
END_DEFINE_SPEC(FNewEnemyCountTest)

void FNewEnemyCountTest::Define()
{
	// 2. BeforeEach - defines what happens before each test.
	BeforeEach([this]()
	{
		// 3. Before each test we want to open a game map.
		AutomationOpenMap(TEXT("/Game/Maps/Map_Grassland01"));

		// 4. Before each test the World is obtained and tested if is valid.
		World = GetTestWorld();
		PController = World->GetFirstPlayerController();

		TArray<USentinelProfiler*> profilerComps;
		PController->GetComponents(profilerComps);
		profiler_component = profilerComps[0];

		TestNotNull("Check if World is properly created", World);
	});

	LatentIt("should return available items", [this](const FDoneDelegate& Done)
	{
		// profiler_component->OnGPUCaptureFinished.Add(Done);
		
		Done.Execute();
		// BackendService->QueryItems(this, &FMyCustomSpec::HandleQueryItemComplete, Done);
	});
	// 5. It - defines one test.
	It("Capture First", [this]()
	{
		// 7. Test if there are 3 enemy characters.
		profiler_component->CaptureGPUData("Fudge");
		TestTrue("Check if there are 3 enemies on the level", true);
	});

	// 8. AfterEach - defines what happens after every test.
	AfterEach([this]()
	{
		// 9. After each test close game map.
		Exit();
	});
}
