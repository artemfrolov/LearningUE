// Learning project - written by hand, not from the template.

#include "EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "StatsComponent.h"
#include "MeleeAttackComponent.h"
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

	// --- hearing ---

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	// How far a noise of full loudness carries, in cm. Loudness MULTIPLIES this, so a
	// noise reported at 0.5 is heard from half as far. That is the whole stealth dial.
	HearingConfig->HearingRange = 2000.0f;

	// Same affiliation trap as sight, same answer.
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// Both senses into one component. It runs them independently and reports both
	// through the single OnTargetPerceptionUpdated delegate.
	Perception->ConfigureSense(*SightConfig);
	Perception->ConfigureSense(*HearingConfig);

	// Which sense wins when two disagree about where a target is. Eyes beat ears: if we
	// can see you, the sound of you somewhere else does not move our estimate.
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
	// Tidiness rather than safety: the timer manager holds a weak reference and will not
	// call into a destroyed object anyway. Stopping our own timer explicitly is still the
	// habit worth having, because not every callback mechanism is that forgiving.
	GetWorld()->GetTimerManager().ClearTimer(ThinkTimer);

	Super::EndPlay(EndPlayReason);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Ask the body what it HAS. The controller does not care whether it is driving an
	// AEnemyCharacter or anything else - only whether the thing it is driving can punch.
	// A body with no attack component simply chases and never swings, which is a perfectly
	// good unarmed enemy.
	MeleeAttack = InPawn ? InPawn->FindComponentByClass<UMeleeAttackComponent>() : nullptr;

	if (!MeleeAttack)
	{
		UE_LOG(LogLearningUE, Warning, TEXT("%s has no MeleeAttackComponent - it will chase but never attack."), *GetNameSafe(InPawn));
	}
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

		if (Target && bTargetVisible)
		{
			// Focus on the ACTOR: the head tracks them as they move.
			SetFocus(Target, EAIFocusPriority::Gameplay);
		}
		else
		{
			// Focus on a POINT: we heard something over there and turn to look at where
			// the sound was. Focusing on the actor here would be a cheat - the body
			// would track a player it has no business knowing the position of.
			SetFocalPoint(LastKnownLocation, EAIFocusPriority::Gameplay);
		}
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

	// One delegate, two senses. FAIStimulus carries which sense produced it, and the two
	// mean genuinely different things - seeing you is evidence of where you ARE, hearing
	// you is evidence of where you WERE a moment ago.
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		HandleHearingUpdated(Actor, Stimulus);
		return;
	}

	HandleSightUpdated(Actor, Stimulus);
}

void AEnemyAIController::HandleSightUpdated(AActor* Actor, const FAIStimulus& Stimulus)
{
	bTargetVisible = Stimulus.WasSuccessfullySensed();

	if (bTargetVisible)
	{
		Target = Actor;
		LastKnownLocation = Actor->GetActorLocation();

		// From here on this alert is an EYES alert, even if ears started it. If we now
		// lose sight, that is a lost target rather than an uninvestigated noise.
		bAlertedBySight = true;

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

void AEnemyAIController::HandleHearingUpdated(AActor* Actor, const FAIStimulus& Stimulus)
{
	// Hearing has no "stopped hearing" event - a noise happens or it does not. Sight
	// reports both directions; ears only ever report success.
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	// Where the SOUND was, which is where the player was when they made it. By the time
	// the enemy walks over there they may be long gone, and that gap is the stealth.
	LastKnownLocation = Stimulus.StimulusLocation;

	if (bShowStateDebug)
	{
		DrawDebugSphere(GetWorld(), LastKnownLocation, 40.0f, 12, FColor::Cyan, false, 2.0f);
	}

	switch (AlertState)
	{
	case EEnemyAlertState::Relaxed:
		// "What was that?" - notice, turn to look, but do not go hunting yet. Note we do
		// NOT set Target: we heard a noise, we did not identify a person.
		bAlertedBySight = false;
		SetAlertState(EEnemyAlertState::Alerted);
		break;

	case EEnemyAlertState::Searching:
		// A fresh noise while hunting restarts the clock and moves the destination. Keep
		// making sound while being searched for and they never give up.
		StateEnteredTime = GetWorld()->GetTimeSeconds();
		break;

	default:
		// Alerted is already counting down; Attacking already has them in view. Neither
		// needs a noise to tell it anything.
		break;
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
		if (bTargetVisible && Target)
		{
			// keep the memory fresh while we can still see them
			LastKnownLocation = Target->GetActorLocation();

			if (TimeInState() >= ConfirmDelay)
			{
				SetAlertState(EEnemyAlertState::Attacking);
			}
			break;
		}

		// Nothing in view. Wait out the same beat, then draw the conclusion the CAUSE
		// calls for: a glimpse that never resolved is dropped, a noise gets investigated.
		if (TimeInState() >= ConfirmDelay)
		{
			SetAlertState(bAlertedBySight ? EEnemyAlertState::Relaxed : EEnemyAlertState::Searching);
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

		// A swing is a commitment for us exactly as it is for the player. Standing still
		// for the whole animation is what gives you a window to step out of reach, and it
		// is the difference between a fight and a shoving match.
		if (MeleeAttack && MeleeAttack->IsAttacking())
		{
			StopMovement();
			break;
		}

		// Re-issue the move order only when we are actually out of range. Firing MoveTo
		// at an unchanged goal every think is how an AI ends up stuttering in place:
		// each order cancels the previous path and starts a new one.
		const float Distance = FVector::Dist(MyPawn->GetActorLocation(), Target->GetActorLocation());

		if (Distance > AttackRange)
		{
			// MoveToActor tracks a MOVING goal - it repaths on its own as the player
			// runs, which MoveToLocation would not do.
			//
			// bStopOnOverlap = FALSE is the important argument. Left at its default of
			// true, path following adds the two capsule radii to the acceptance radius
			// "so the bodies just touch" - so asking for 150 actually stopped the enemy
			// at about 234cm, permanently outside the reach of its own fist. With it
			// off, the number means the distance it says.
			MoveToActor(Target, CombatApproachDistance, /*bStopOnOverlap=*/ false);
			break;
		}

		StopMovement();

		// --- in reach: should we swing? ---

		// Only at something we can actually SEE. Without this the enemy would punch the
		// remembered position of a player standing behind a wall, because Attacking keeps
		// its target for a moment after sight is lost.
		if (!bTargetVisible)
		{
			break;
		}

		if (!MeleeAttack)
		{
			break;
		}

		const float Now = GetWorld()->GetTimeSeconds();

		if (Now - LastAttackTime < AttackCooldown)
		{
			break;
		}

		// StartLightAttack can still refuse - no weapon, no stamina, already swinging.
		// Only record the time if a punch actually happened, or a refused swing would
		// start the cooldown and the enemy would stand there paying for nothing.
		if (MeleeAttack->StartLightAttack())
		{
			LastAttackTime = Now;
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
