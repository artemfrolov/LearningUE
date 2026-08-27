// Learning project - written by hand, not from the template.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_AttackHit.generated.h"

/**
 *  Placed on an attack montage at the frame where the blow lands. When playback
 *  reaches it, it tells the owning character to check what is in front of it.
 *
 *  The notify carries no combat logic of its own - it is a marker that knows how to
 *  shout. Damage, range and what counts as a target all live on the character.
 */
UCLASS(meta = (DisplayName = "Attack Hit"))
class UAnimNotify_AttackHit : public UAnimNotify
{
	GENERATED_BODY()

protected:

	/**
	 *  Which bone or socket the blow comes from. Editable per notify, so the same class
	 *  serves a right hook, a left jab, and later a sword socket - only this string changes.
	 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	FName AttackBoneName = FName("hand_r");

public:

	/** Called by the animation system when playback passes this marker */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** The label shown on the montage timeline */
	virtual FString GetNotifyName_Implementation() const override;
};
