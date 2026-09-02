// Learning project - written by hand, not from the template.

#include "AnimNotify_AttackHit.h"
#include "Components/SkeletalMeshComponent.h"
#include "MeleeAttackComponent.h"

void UAnimNotify_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// The notify is handed the MESH, so the actor it belongs to is one step up.
	//
	// This used to cast to ALearningUECharacter, with a note that the enemy needing to
	// swing would be the moment to generalise. That moment arrived. Asking what the actor
	// HAS rather than what it IS means one notify serves the player, the enemy, and
	// anything else that ever grows an attack component - including actors that share no
	// base class at all.
	if (AActor* Attacker = MeshComp->GetOwner())
	{
		if (UMeleeAttackComponent* Attack = Attacker->FindComponentByClass<UMeleeAttackComponent>())
		{
			Attack->DoAttackTrace(AttackBoneName);
		}
	}
}

FString UAnimNotify_AttackHit::GetNotifyName_Implementation() const
{
	// what shows on the montage timeline, so it can be identified at a glance
	return FString("Attack Hit");
}
