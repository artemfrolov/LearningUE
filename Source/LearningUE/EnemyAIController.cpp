// Learning project - written by hand, not from the template.

#include "EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "LearningUE.h"

AEnemyAIController::AEnemyAIController()
{
	// The component that RUNS senses. On its own it senses nothing - it is the engine
	// underneath, and a sense has to be configured into it.
	Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));

	// The sight sense's settings, as their own object.
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// How far we can see, in cm. 15 metres.
	SightConfig->SightRadius = 1500.0f;

	// How far you must get before we LOSE you - deliberately larger than SightRadius.
	// If the two were equal, standing exactly on the boundary would spam seen/lost/seen
	// every time you shifted a centimetre. The gap between them is hysteresis: it costs
	// nothing and it is the difference between a stable AI and a twitching one.
	SightConfig->LoseSightRadius = 1800.0f;

	// HALF the cone, despite the name. 60 here means a 120-degree field of view, which
	// is roughly human. Set this to 360 expecting all-round vision and you get an AI
	// that sees through the back of its own head, twice over.
	SightConfig->PeripheralVisionAngleDegrees = 60.0f;

	// How long a sighting stays "current" after the sense stops reporting it. This is
	// the AI's short-term memory, and in 5.2 it is what lets a searching enemy keep
	// looking at where you WERE rather than instantly forgetting you.
	SightConfig->SetMaxAge(5.0f);

	// Anything closer than this is seen regardless of the cone - you cannot sneak up on
	// someone by standing on their toes. -1 disables it; we want the behaviour, at 3m.
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;

	// THE TRAP. Sight filters what it reports by team affiliation, and by default only
	// reports ENEMIES. But "enemy" requires a team system (IGenericTeamAgentInterface),
	// and without one every actor in the game is NEUTRAL - so the default configuration
	// detects precisely nothing, silently. This is the single most common reason people
	// conclude AIPerception is broken.
	//
	// We have no teams, so we detect everything and sort it out ourselves.
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// Hand the configured sense to the component, and say which sense wins when several
	// disagree about where a target is. With one sense it changes nothing; with hearing
	// added in 5.3 it decides whether seeing you or hearing you is the better fix.
	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());

	// AAIController has a PerceptionComponent slot of its own. Filling it is what makes
	// engine code - and the Gameplay Debugger - find our senses.
	SetPerceptionComponent(*Perception);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	// UE idiom: subscribe rather than poll. The component decides when something changed
	// and calls us; we never ask "can I see him yet?" on a timer.
	Perception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::HandlePerceptionUpdated);
}

void AEnemyAIController::HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	// One event covers both directions. WasSuccessfullySensed() is true when the target
	// came INTO view and false when it dropped out - the same delegate, opposite meaning,
	// which is easy to miss and produces an AI that never stops chasing.
	const bool bSeen = Stimulus.WasSuccessfullySensed();

	// Where the stimulus came from. On a loss this is the LAST place we saw them, not
	// where they are now - which is exactly what a searching state needs in 5.2.
	const FVector StimulusLocation = Stimulus.StimulusLocation;
	const float Distance = FVector::Dist(GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector, StimulusLocation);

	UE_LOG(LogLearningUE, Warning, TEXT("%s %s %s at %.0fcm"),
		*GetNameSafe(GetPawn()),
		bSeen ? TEXT("SEES") : TEXT("LOST SIGHT OF"),
		*GetNameSafe(Actor),
		Distance);
}
