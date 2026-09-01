// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 *  The brain for AEnemyCharacter.
 *
 *  A Controller is a separate ACTOR from the pawn it drives. The enemy character is a
 *  body - mesh, capsule, health - and this is what decides what that body does. Exactly
 *  the relationship APlayerController has with the player's character, which is why
 *  possessing an enemy with a PlayerController would let you drive it yourself.
 *
 *  Phase 5.1 gives it one sense and no behaviour: it can see, and it says so. What it
 *  does about it comes in 5.2.
 */
UCLASS()
class AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:

	/** Constructor - builds the perception component and configures the sight sense */
	AEnemyAIController();

protected:

	/** Subscribe to perception events once we exist in the world */
	virtual void BeginPlay() override;

	/**
	 *  The perception component: the machinery that runs our senses and reports what
	 *  they found. It owns no senses by itself - a sense is configured into it.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* Perception;

	/**
	 *  The sight sense's settings. A separate object rather than fields on the component
	 *  because one perception component can run several senses at once - sight, hearing,
	 *  damage - each with its own configuration. Hearing arrives in 5.3 as a second one
	 *  of these.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	/**
	 *  Called by the perception component whenever something is seen or stops being
	 *  seen. UFUNCTION is mandatory - this binds to a dynamic delegate, which looks
	 *  functions up by name at runtime.
	 *
	 *  Fires on CHANGE, not continuously. One call when you come into view, one when you
	 *  drop out of it. That is the whole reason to use perception instead of tracing at
	 *  the player every frame.
	 */
	UFUNCTION()
	void HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
