#include "AppPrecomp.h"
#include "StateFrozen.h"
#include "MovingEntity.h"

StateFrozen registryInstanceFrozen(NULL); //self register ourselves in the brain registry

StateFrozen::StateFrozen(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateFrozen::~StateFrozen()
{
}

void StateFrozen::OnAdd()
{
}

void StateFrozen::OnRemove()
{
}

void StateFrozen::Update(float step)
{

}

void StateFrozen::PostUpdate(float step)
{
	if (AnimIsLooping())
	{
		//this resets the animation, just play forever
	}
}

/*
Object: Frozen
A state that causes an entity to stop moving.  Does not change animation from whatever it was.

Usage:

(code)
if (GameIsActive() == false) then
	//no profile is active, so we must be in the editor.  Let's not have them running around
	this:GetBrainManager():SetStateByName("Frozen");
end
(end)
*/