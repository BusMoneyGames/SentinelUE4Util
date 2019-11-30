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

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSentinelProfilingCommand, FString, Command);
bool FSentinelProfilingCommand::Update()
{
	UWorld* world = GetTestWorld();

	APlayerController* PController = world->GetFirstPlayerController();

	TArray<USentinelProfiler*> profilerComps;
	PController->GetComponents(profilerComps);
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(5.0f));
	USentinelProfiler* profiler_component = profilerComps[0];
	profiler_component->CaptureGPUData("Code_Test");

	UE_LOG(LogTemp, Warning, TEXT("Latent command name: %s"), *Command);

	return profiler_component->isProfiling;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSentinelAutomationCommandlet, "Sentinel.Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSentinelAutomationCommandlet::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Medieval_Armory/Maps/Demo_01"));
	
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));


	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("setres 640x480")));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f));


	ADD_LATENT_AUTOMATION_COMMAND(FSentinelProfilingCommand("adfasdf"));


    return true;
}
