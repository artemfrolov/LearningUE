// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

/**
 *  How aware of the player this enemy is. The four states from the design doc.
 *
 *  Deliberately about KNOWLEDGE, not about action. "Searching" describes what the enemy
 *  believes, and walking to the last known location is merely what it does about that
 *  belief. Keeping the two apart is what will let a future enemy type search by standing
 *  still and shouting for help instead.
 */
UENUM(BlueprintType)
enum class EEnemyAlertState : uint8
{
	/** Nothing has been noticed. Idle. */
	Relaxed		UMETA(DisplayName = "Relaxed"),

	/** Something was noticed but not yet committed to. A countdown is running. */
	Alerted		UMETA(DisplayName = "Alerted"),

	/** Had the player, lost them. Heading for where they were last seen. */
	Searching	UMETA(DisplayName = "Searching"),

	/** Committed. Closing to melee range. */
	Attacking	UMETA(DisplayName = "Attacking")
};

/**
 *  The brain for AEnemyCharacter.
 *
 *  A Controller is a separate ACTOR from the pawn it drives. The enemy character is a
 *  body - mesh, capsule, health - and this is what decides what that body does. Exactly
 *  the relationship APlayerController has with the player's character.
 */
UCLASS()
class AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:

	/** Constructor - builds the perception component and configures the sight sense */
	AEnemyAIController();

	/** What this enemy currently believes. Exposed for debug displays and future HUD. */
	UFUNCTION(BlueprintPure, Category = "AI")
	EEnemyAlertState GetAlertState() const { return AlertState; }

protected:

	/** Subscribe to perception and start the think timer */
	virtual void BeginPlay() override;

	/** Stop the think timer when this controller goes away */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- senses ---

	/**
	 *  The machinery that runs our senses and reports what they found. It owns no senses
	 *  by itself - a sense is configured into it.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* Perception;

	/**
	 *  The sight sense's settings. A separate object because one perception component can
	 *  run several senses at once, each configured independently. Hearing arrives in 5.3
	 *  as a second one of these.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	/**
	 *  The hearing sense's settings. Second sense on the SAME perception component -
	 *  that is what the component is for. Both report through one delegate, told apart
	 *  by the stimulus type.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Hearing* HearingConfig;

	/**
	 *  Called whenever something is seen or stops being seen. Fires on CHANGE, not
	 *  continuously - one call in, one call out.
	 */
	UFUNCTION()
	void HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** A sighting gained or lost. Can lead to an attack. */
	void HandleSightUpdated(AActor* Actor, const FAIStimulus& Stimulus);

	/** A noise heard. Never leads directly to an attack - only to looking. */
	void HandleHearingUpdated(AActor* Actor, const FAIStimulus& Stimulus);

	// --- state ---

	/** What we currently believe. Runtime state, so read-only in the editor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	EEnemyAlertState AlertState = EEnemyAlertState::Relaxed;

	/**
	 *  Who we are interested in. UPROPERTY so the garbage collector knows we hold this
	 *  reference, and so it is nulled rather than dangling if that actor is destroyed.
	 */
	UPROPERTY()
	AActor* Target = nullptr;

	/** True only while sight is actually reporting the target right now. */
	bool bTargetVisible = false;

	/**
	 *  Whether the current alert began with EYES rather than ears.
	 *
	 *  It decides what an unresolved alert becomes. A glimpse that never turned into a
	 *  proper sighting is shrugged off; a noise with nobody in view is worth walking
	 *  over to look at. Same state, same countdown, opposite conclusions.
	 */
	bool bAlertedBySight = false;

	/**
	 *  Where we last had them. Not where they are - that is the entire point. This is the
	 *  difference between an enemy that searches and an enemy that cheats.
	 */
	FVector LastKnownLocation = FVector::ZeroVector;

	/** When the current state began, for the two timed transitions. */
	float StateEnteredTime = 0.0f;

	/** Change state, remember when, and act on the entry. The only way state changes. */
	void SetAlertState(EEnemyAlertState NewState);

	/** Seconds spent in the current state. */
	float TimeInState() const;

	// --- thinking ---

	/**
	 *  Runs the current state, on a timer rather than every frame. An AI deciding things
	 *  five times a second is indistinguishable from sixty and costs a twelfth as much.
	 */
	void Think();

	FTimerHandle ThinkTimer;

	/** How often Think runs, in seconds. */
	UPROPERTY(EditAnywhere, Category = "AI|Tuning", meta = (ClampMin = "0.02"))
	float ThinkInterval = 0.2f;

	// --- tuning ---

	/**
	 *  Seconds of continuous sight before Alerted becomes Attacking. The grace period:
	 *  break line of sight inside it and the enemy never commits.
	 */
	UPROPERTY(EditAnywhere, Category = "AI|Tuning", meta = (ClampMin = "0.0"))
	float ConfirmDelay = 1.5f;

	/** Seconds spent searching before giving up and relaxing. */
	UPROPERTY(EditAnywhere, Category = "AI|Tuning", meta = (ClampMin = "0.0"))
	float SearchDuration = 6.0f;

	/** How close to get before stopping, in cm. Melee reach, roughly. */
	UPROPERTY(EditAnywhere, Category = "AI|Tuning", meta = (ClampMin = "0.0"))
	float AttackRange = 150.0f;

	/** How close counts as "arrived" when walking to the last known location, in cm. */
	UPROPERTY(EditAnywhere, Category = "AI|Tuning", meta = (ClampMin = "0.0"))
	float SearchAcceptanceRadius = 100.0f;

	/** Draw the current state above the enemy's head. Turn off before packaging. */
	UPROPERTY(EditAnywhere, Category = "AI|Debug")
	bool bShowStateDebug = true;

	/** Paints the state name over the pawn. Called from Think. */
	void DrawStateDebug() const;
};
