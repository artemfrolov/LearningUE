// Learning project - written by hand, not from the template.

#include "EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "StatsComponent.h"
#include "LearningUE.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

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

	// How long a sighting stays "current" after the sense stops reporting it.
	SightConfig->SetMaxAge(5.0f);

	// Once we have ALREADY seen you, staying within 3m of where we last saw you keeps
	// you visible regardless of the cone. Note the "LastSeen" in the name: this can only
	// prevent LOSING a target, never find one. Walking up behind an unaware enemy and
	// touching it stays undetected, which is the backstab window.
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;

	// THE TRAP. Sight filters what it reports by team affiliation, and by default only
	// reports ENEMIES. But "enemy" requires a team system (IGenericTeamAgentInterface),
	// and without one every actor in the game is NEUTRAL - so the default configuration
	// detects precisely nothing, silently.
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

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

	// The AI's heartbeat. Deciding, not animating - 5Hz is plenty.
	GetWorld()->GetTimerManager().SetTimer(ThinkTimer, this, &AEnemyAIController::Think, ThinkInterval, true);

	StateEnteredTime = GetWorld()->GetTimeSeconds();
}

void AEnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// A timer outlives the object that scheduled it unless you say otherwise, and a
	// timer firing into a destroyed controller is a crash.
	GetWorld()->GetTimerManager().ClearTimer(ThinkTimer);

	Super::EndPlay(EndPlayReason);
}

float AEnemyAIController::TimeInState() const
{
	return GetWorld()->GetTimeSeconds() - StateEnteredTime;
}

void AEnemyAIController::SetAlertState(EEnemyAlertState NewState)
{
	if (AlertState == NewState)
	{
		return;
	}

	AlertState = NewState;
	StateEnteredTime = GetWorld()->GetTimeSeconds();

	// Entry actions. Everything that must happen ONCE on arriving in a state lives here;
	// everything that must happen repeatedly lives in Think. Mixing the two is how state
	// machines rot - a move order re-issued sixty times a second never gets anywhere.
	switch (AlertState)
	{
	case EEnemyAlertState::Relaxed:
		Target = nullptr;
		bTargetVisible = false;
		ClearFocus(EAIFocusPriority::Gameplay);
		StopMovement();
		break;

	case EEnemyAlertState::Alerted:
		// stand still and stare while the countdown runs - the visible "did I see
		// something?" beat that gives the player a chance to break away
		StopMovement();
		SetFocus(Target, EAIFocusPriority::Gameplay);
		break;

	case EEnemyAlertState::Searching:
		// stop staring at a target we can no longer see, so the body faces where it
		// walks instead of at a memory
		ClearFocus(EAIFocusPriority::Gameplay);
		break;

	case EEnemyAlertState::Attacking:
		SetFocus(Target, EAIFocusPriority::Gameplay);
		break;
	}

	UE_LOG(LogLearningUE, Warning, TEXT("%s -> %s"),
		*GetNameSafe(GetPawn()),
		*StaticEnum<EEnemyAlertState>()->GetNameStringByValue(static_cast<int64>(AlertState)));
}

void AEnemyAIController::HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	// Only the player interests us. Without this, two enemies standing in sight of each
	// other would each see a pawn, alert, and charge one another - because with no team
	// system every actor is neutral and sight reports everything.
	//
	// This is a stand-in for teams (IGenericTeamAgentInterface), which is where this
	// check properly belongs once there is more than one faction.
	const APawn* SeenPawn = Cast<APawn>(Actor);

	if (!SeenPawn || !SeenPawn->IsPlayerControlled())
	{
		return;
	}

	bTargetVisible = Stimulus.WasSuccessfullySensed();

	if (bTargetVisible)
	{
		Target = Actor;
		LastKnownLocation = Actor->GetActorLocation();

		switch (AlertState)
		{
		case EEnemyAlertState::Relaxed:
			// first contact - notice, but do not commit yet
			SetAlertState(EEnemyAlertState::Alerted);
			break;

		case EEnemyAlertState::Searching:
			// Already hunting, so no second grace period. Re-spotting someone you were
			// looking for is not the same as noticing a stranger.
			SetAlertState(EEnemyAlertState::Attacking);
			break;

		default:
			// Alerted keeps counting down; Attacking is already there
			break;
		}
	}
	else
	{
		// On a LOSS this is where the target was last sensed, not where it is now -
		// exactly the memory a search needs.
		LastKnownLocation = Stimulus.StimulusLocation;

		switch (AlertState)
		{
		case EEnemyAlertState::Alerted:
			// never confirmed, so nothing to search for - shrug it off
			SetAlertState(EEnemyAlertState::Relaxed);
			break;

		case EEnemyAlertState::Attacking:
			SetAlertState(EEnemyAlertState::Searching);
			break;

		default:
			break;
		}
	}
}

void AEnemyAIController::Think()
{
	APawn* MyPawn = GetPawn();

	if (!MyPawn)
	{
		return;
	}

	// A corpse does not think. The AI is a separate actor from the body, so nothing
	// stops it giving orders to a ragdoll unless we check.
	if (const UStatsComponent* MyStats = MyPawn->FindComponentByClass<UStatsComponent>())
	{
		if (!MyStats->IsAlive())
		{
			StopMovement();
			GetWorld()->GetTimerManager().ClearTimer(ThinkTimer);
			return;
		}
	}

	// The target may have died or been destroyed since we last looked. UPROPERTY nulls
	// a destroyed actor for us; a dead-but-present one has to be asked.
	if (Target)
	{
		const UStatsComponent* TargetStats = Target->FindComponentByClass<UStatsComponent>();

		if (TargetStats && !TargetStats->IsAlive())
		{
			SetAlertState(EEnemyAlertState::Relaxed);
		}
	}

	switch (AlertState)
	{
	case EEnemyAlertState::Relaxed:
		// nothing to do. A patrol route would go here.
		break;

	case EEnemyAlertState::Alerted:
		if (!bTargetVisible || !Target)
		{
			// perception already handles the loss; this catches a destroyed target
			SetAlertState(EEnemyAlertState::Relaxed);
			break;
		}

		// keep the memory fresh while we can still see them
		LastKnownLocation = Target->GetActorLocation();

		if (TimeInState() >= ConfirmDelay)
		{
			SetAlertState(EEnemyAlertState::Attacking);
		}
		break;

	case EEnemyAlertState::Attacking:
	{
		if (!Target)
		{
			SetAlertState(EEnemyAlertState::Searching);
			break;
		}

		if (bTargetVisible)
		{
			LastKnownLocation = Target->GetActorLocation();
		}

		// Re-issue the move order only when we are actually out of range. Firing MoveTo
		// at an unchanged goal every think is how an AI ends up stuttering in place:
		// each order cancels the previous path and starts a new one.
		const float Distance = FVector::Dist(MyPawn->GetActorLocation(), Target->GetActorLocation());

		if (Distance > AttackRange)
		{
			// MoveToActor tracks a MOVING goal - it repaths on its own as the player
			// runs, which MoveToLocation would not do.
			MoveToActor(Target, AttackRange);
		}
		else
		{
			// in reach. There is nothing to hit with yet - the enemy has no attack until
			// 5.4 - so it closes and waits.
			StopMovement();
		}
		break;
	}

	case EEnemyAlertState::Searching:
		if (TimeInState() >= SearchDuration)
		{
			SetAlertState(EEnemyAlertState::Relaxed);
			break;
		}

		// Walk to the memory, not to the player. If this said Target->GetActorLocation()
		// the enemy would track you through walls and the whole detection system would
		// be decoration.
		MoveToLocation(LastKnownLocation, SearchAcceptanceRadius);
		break;
	}

	DrawStateDebug();
}

void AEnemyAIController::DrawStateDebug() const
{
	if (!bShowStateDebug || !GetPawn())
	{
		return;
	}

	FColor Colour = FColor::White;

	switch (AlertState)
	{
	case EEnemyAlertState::Relaxed:		Colour = FColor::Green;		break;
	case EEnemyAlertState::Alerted:		Colour = FColor::Yellow;	break;
	case EEnemyAlertState::Searching:	Colour = FColor::Orange;	break;
	case EEnemyAlertState::Attacking:	Colour = FColor::Red;		break;
	}

	const FString Label = StaticEnum<EEnemyAlertState>()->GetNameStringByValue(static_cast<int64>(AlertState));
	const FVector Above = GetPawn()->GetActorLocation() + FVector(0.0f, 0.0f, 120.0f);

	// Lifetime matches the think interval, so each label replaces the last instead of
	// stacking up into an unreadable smear.
	DrawDebugString(GetWorld(), Above, Label, nullptr, Colour, ThinkInterval, true);

	// Where the enemy BELIEVES the player is. When this sphere and the player separate,
	// you are looking at the AI being wrong - which is the entire point of the system.
	if (AlertState == EEnemyAlertState::Searching)
	{
		DrawDebugSphere(GetWorld(), LastKnownLocation, 50.0f, 12, FColor::Orange, false, ThinkInterval);
	}
}
