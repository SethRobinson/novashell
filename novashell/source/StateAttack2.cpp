#include "AppPrecomp.h"
#include "StateAttack2.h"
#include "MovingEntity.h"

StateAttack2 registryInstanceStateAttack2(NULL); //self register ourselves i nthe brain registry

StateAttack2::StateAttack2(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateAttack2::~StateAttack2()
{
}

void StateAttack2::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_ATTACK2);
}

void StateAttack2::OnRemove()
{
}

void StateAttack2::Update(float step)
{

}

void StateAttack2::PostUpdate(float step)
{
	if (AnimIsLooping() && m_pParent->GetScriptObject()->FunctionExists("OnAttack2Loop"))
	{
		m_pParent->RunFunction("OnAttack2Loop");
	}
}

/*
Object: Attack2
A state that causes an entity to stop moving and play his attack animation.


An attack animation is detected from the visual profile .xml when anim states named <attack2_left>, <attack2_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <attack2_left>.

Script functions it calls:

If the script function *"OnAttack2Loop"* exists in the entity's script, it will call it every time the animation loops.
*/
