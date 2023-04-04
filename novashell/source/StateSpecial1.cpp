#include "AppPrecomp.h"
#include "StateSpecial1.h"
#include "MovingEntity.h"

StateSpecial1 registryInstanceStateSpecial1(NULL); //self register ourselves i nthe brain registry

StateSpecial1::StateSpecial1(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateSpecial1::~StateSpecial1()
{
}

void StateSpecial1::OnAdd()
{
	
	if (m_pParent->GetVisualProfile())
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_SPECIAL1);
	}
}

void StateSpecial1::OnRemove()
{
}

void StateSpecial1::Update(float step)
{

}

void StateSpecial1::PostUpdate(float step)
{
	if (AnimIsLooping() && m_pParent->GetScriptObject()->FunctionExists("OnSpecial1Loop"))
	{
		m_pParent->RunFunction("OnSpecial1Loop");
	}
}

/*
Object: Special1
A state that causes an entity to stop moving and play his attack animation.


An attack animation is detected from the visual profile .xml when anim states named <special1_left>, <special1_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <special1_left>.

Script functions it calls:

If the script function *"OnSpecial1Loop"* exists in the entity's script, it will call it every time the animation loops.
*/
