#include "AppPrecomp.h"
#include "StateTopWalk.h"
#include "MovingEntity.h"

StateTopWalk registryInstanceStateTopWalk(NULL); //self register ourselves in the brain registry

StateTopWalk::StateTopWalk(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTopWalk::~StateTopWalk()
{
}

void StateTopWalk::OnAdd()
{
	if (m_pParent->GetVisualProfile())
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_WALK);
	}

	m_bCallbackActive = m_pParent->GetScriptObject()->FunctionExists("OnWalkLoop");
}

void StateTopWalk::OnRemove()
{
}

void StateTopWalk::Update(float step)
{
	if (m_pParent->IsFacingTarget(0.5))
	{
		m_pParent->GetBrainManager()->GetBrainBase()->AddWeightedForce( m_pParent->GetVectorFacing() * m_pParent->GetDesiredSpeed() );
	}
}

void StateTopWalk::PostUpdate(float step)
{
	if (m_bCallbackActive && AnimIsLooping())
	{
		m_pParent->RunFunction("OnWalkLoop");
	}
}


/*
Object: Walk
A state that causes an entity to play his walk animation and move in the direction he's current facing at the speed set in <Entity::SetDesiredSpeed>.

An Walk animation is detected from the visual profile .xml when anim states named <walk_left>, <walk_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <walk_left>.

Script functions it calls:

If the script function *"OnWalkLoop"* exists in the entity's script, it will call it every time the animation loops.
*/
