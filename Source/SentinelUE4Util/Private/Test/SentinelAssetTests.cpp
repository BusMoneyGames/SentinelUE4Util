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

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSentinelProfilingCommand, FString, Command);
bool FSentinelProfilingCommand::Update()
{
	UWorld* world = GetTestWorld();

	APlayerController* PController = world->GetFirstPlayerController();

	TArray<USentinelPCComponent*> profilerComps;
	PController->GetComponents(profilerComps);

	USentinelPCComponent* profiler_component = profilerComps[0];

	if (!profiler_component->isProfiling)
	{
		profiler_component->CaptureGPUData(Command);
	}

	return profiler_component->isProfiling;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FMoveToNextTestLocation);
bool FMoveToNextTestLocation::Update()
{
	UWorld* world = GetTestWorld();

	for (TActorIterator<ASentinelTestLocation> ActorItr(world); ActorItr; ++ActorItr)
	{
		ASentinelTestLocation* test_actor = Cast<ASentinelTestLocation>(*ActorItr);
		if (test_actor)
		{
			if (APlayerController* TargetPC = UGameplayStatics::GetPlayerController(world, 0))
			{
				TargetPC->GetPawn()->SetActorLocation(test_actor->GetActorLocation());
			}
		}
	}

	return true;
}


IMPLEMENT_COMPLEX_AUTOMATION_TEST(FSentinelAutomationCommandlet, "Sentinel.Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FSentinelAutomationCommandlet::GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const
{

	OutBeautifiedNames.Add("Name1");
	OutTestCommands.Add("Command");
}

bool FSentinelAutomationCommandlet::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Medieval_Armory/Maps/Demo_01"));
	
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FMoveToNextTestLocation);

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FSentinelProfilingCommand("Fun-Hardcoded"));

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);

    return true;
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
	// 2. BeforeEach - defines what happens before each test.
	BeforeEach([this]()
	{
		// 3. Before each test we want to open a game map.
		AutomationOpenMap(TEXT("/Game/Medieval_Armory/Maps/Demo_01"));

		// 4. Before each test the World is obtained and tested if is valid.
		World = GetTestWorld();
		PController = World->GetFirstPlayerController();

		TArray<USentinelPCComponent*> profilerComps;
		PController->GetComponents(profilerComps);
		profiler_component = profilerComps[0];

		TestNotNull("Check if World is properly created", World);
	});

	LatentIt("should return available items", [this](const FDoneDelegate& Done)
	{
		// profiler_component->OnGPUCaptureFinished.AddUnique();
		
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
