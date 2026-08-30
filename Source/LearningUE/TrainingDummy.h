// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainingDummy.generated.h"

class UStaticMeshComponent;
class UStatsComponent;

/**
 *  A punching bag.
 *
 *  Note what it is NOT: not a Character, not a Pawn, no skeleton, no legs, no movement
 *  component. It is a plain AActor with a mesh and a UStatsComponent bolted on - and it
 *  takes damage and dies through exactly the same code as the player.
 *
 *  This is the argument from the start of Phase 2, made executable. Health could never
 *  have been inherited from a shared parent, because this class and ALearningUECharacter
 *  have no common ancestor below AActor. Attached, not inherited.
 */
UCLASS(abstract)
class ATrainingDummy : public AActor
{
	GENERATED_BODY()

public:

	/** Constructor */
	ATrainingDummy();

protected:

	/** The visible body. Which mesh asset it uses is the Blueprint's business, not ours. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	/** The same component the player carries */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStatsComponent* Stats;

	/** Subscribe to the stats events */
	virtual void BeginPlay() override;

	/** Logs the new health so the dummy can be watched without a HUD */
	UFUNCTION()
	void HandleHealthChanged(float NewValue, float MaxValue);

	/** Removes the dummy from the world */
	UFUNCTION()
	void HandleDeath(AActor* Killer);
};
