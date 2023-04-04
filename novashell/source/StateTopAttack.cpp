#include "AppPrecomp.h"
#include "StateTopAttack.h"
#include "MovingEntity.h"

StateTopAttack registryInstanceStateTopAttack(NULL); //self register ourselves i nthe brain registry

StateTopAttack::StateTopAttack(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTopAttack::~StateTopAttack()
{
}

void StateTopAttack::OnAdd()
{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_ATTACK1);
}

void StateTopAttack::OnRemove()
{
}

void StateTopAttack::Update(float step)
{
	
}

void StateTopAttack::PostUpdate(float step)
{
	if (AnimIsLooping() && m_pParent->GetScriptObject()->FunctionExists("OnAttackLoop"))
	{
		m_pParent->RunFunction("OnAttackLoop");
	}
}

/*
Object: Attack
A state that causes an entity to stop moving and play his attack animation.


An attack animation is detected from the visual profile .xml when anim states named <attack1_left>, <attack1_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <attack1_left>.

Script functions it calls:

If the script function *"OnAttackLoop"* exists in the entity's script, it will call it every time the animation loops.
*/
