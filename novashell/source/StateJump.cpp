#include "AppPrecomp.h"
#include "StateJump.h"
#include "MovingEntity.h"

StateJump registryInstanceStateJump(NULL); //self register ourselves i nthe brain registry

StateJump::StateJump(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateJump::~StateJump()
{
}

void StateJump::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_JUMP);
		m_bCallbackActive = m_pParent->GetScriptObject()->FunctionExists("OnJumpLoop");
}

void StateJump::OnRemove()
{
}

void StateJump::Update(float step)
{

	if (m_pParent->IsFacingTarget(0.5))
	{
		m_pParent->GetBrainManager()->GetBrainBase()->AddWeightedForce( m_pParent->GetVectorFacing() * m_pParent->GetDesiredSpeed() );
	}
	
}

void StateJump::PostUpdate(float step)
{
	if (m_bCallbackActive && AnimIsLooping())
	{
		m_pParent->RunFunction("OnJumpLoop");
	}
}

/*
Object: Jump
A state that causes an entity to play his jump animation.

An attack animation is detected from the visual profile .xml when anim states named <jump_left>, <jump_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <jump_left>.

Script functions it calls:

If the script function *"OnJumpLoop"* exists in the entity's script, it will call it every time the animation loops.
*/
