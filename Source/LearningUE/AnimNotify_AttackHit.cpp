// Learning project - written by hand, not from the template.

#include "AnimNotify_AttackHit.h"
#include "Components/SkeletalMeshComponent.h"
#include "LearningUECharacter.h"

void UAnimNotify_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// The notify is handed the MESH, so the actor it belongs to is one step up.
	// Epic's version casts to an interface here because three different classes can
	// attack. We have one, so a direct cast is honest. When the enemy needs to swing
	// in Phase 5, that is the moment an interface starts earning its keep.
	if (ALearningUECharacter* Character = Cast<ALearningUECharacter>(MeshComp->GetOwner()))
	{
		Character->DoAttackTrace(AttackBoneName);
	}
}

FString UAnimNotify_AttackHit::GetNotifyName_Implementation() const
{
	// what shows on the montage timeline, so it can be identified at a glance
	return FString("Attack Hit");
}
