#include "AppPrecomp.h"
#include "VisualProfile.h"
#include "VisualResource.h"
#include "GameLogic.h"

VisualProfile::VisualProfile()
{
	//build up our default anim data, helps us to quickly understand what NPC's can and can't support, custom
	//data can also be added by its profile .xml file

	m_animArray.resize(BUILTIN_ANIM_COUNT);

	m_animArray[IDLE_LEFT].m_name = "idle_left";
	m_animArray[IDLE_RIGHT].m_name = "idle_right";

	m_animArray[IDLE_UP].m_name = "idle_up";
	m_animArray[IDLE_DOWN].m_name = "idle_down";

	m_animArray[IDLE_UP_LEFT].m_name = "idle_up_left";
	m_animArray[IDLE_DOWN_LEFT].m_name = "idle_down_left";

	m_animArray[IDLE_UP_RIGHT].m_name = "idle_up_right";
	m_animArray[IDLE_DOWN_RIGHT].m_name = "idle_down_right";

	m_animArray[WALK_LEFT].m_name = "walk_left";
	m_animArray[WALK_RIGHT].m_name = "walk_right";

	m_animArray[WALK_UP].m_name = "walk_up";
	m_animArray[WALK_DOWN].m_name = "walk_down";

	m_animArray[WALK_UP_LEFT].m_name = "walk_up_left";
	m_animArray[WALK_DOWN_LEFT].m_name = "walk_down_left";

	m_animArray[WALK_UP_RIGHT].m_name = "walk_up_right";
	m_animArray[WALK_DOWN_RIGHT].m_name = "walk_down_right";

	m_animArray[RUN_LEFT].m_name = "run_left";
	m_animArray[RUN_RIGHT].m_name = "run_right";

	m_animArray[RUN_UP].m_name = "run_up";
	m_animArray[RUN_DOWN].m_name = "run_down";

	m_animArray[RUN_UP_LEFT].m_name = "run_up_left";
	m_animArray[RUN_DOWN_LEFT].m_name = "run_down_left";

	m_animArray[RUN_UP_RIGHT].m_name = "run_up_right";
	m_animArray[RUN_DOWN_RIGHT].m_name = "run_down_right";

	m_animArray[PAIN_LEFT].m_name = "pain_left";
	m_animArray[PAIN_RIGHT].m_name = "pain_right";
	m_animArray[PAIN_UP].m_name = "pain_up";
	m_animArray[PAIN_DOWN].m_name = "pain_down";

	m_animArray[PAIN_UP_LEFT].m_name = "pain_up_left";
	m_animArray[PAIN_DOWN_LEFT].m_name = "pain_down_left";

	m_animArray[PAIN_UP_RIGHT].m_name = "pain_up_right";
	m_animArray[PAIN_DOWN_RIGHT].m_name = "pain_down_right";

	m_animArray[DIE_LEFT].m_name = "die_left";
	m_animArray[DIE_RIGHT].m_name = "die_right";
	m_animArray[DIE_UP].m_name = "die_up";
	m_animArray[DIE_DOWN].m_name = "die_down";
	m_animArray[DIE_UP_LEFT].m_name = "die_up_left";
	m_animArray[DIE_DOWN_LEFT].m_name = "die_down_left";

	m_animArray[DIE_UP_RIGHT].m_name = "die_up_right";
	m_animArray[DIE_DOWN_RIGHT].m_name = "die_down_right";
	
	m_animArray[ATTACK1_LEFT].m_name = "attack1_left";
	m_animArray[ATTACK1_RIGHT].m_name = "attack1_right";

	m_animArray[ATTACK1_UP].m_name = "attack1_up";
	m_animArray[ATTACK1_DOWN].m_name = "attack1_down";

	m_animArray[ATTACK1_UP_LEFT].m_name = "attack1_up_left";
	m_animArray[ATTACK1_DOWN_LEFT].m_name = "attack1_down_left";

	m_animArray[ATTACK1_UP_RIGHT].m_name = "attack1_up_right";
	m_animArray[ATTACK1_DOWN_RIGHT].m_name = "attack1_down_right";

	m_animArray[ATTACK2_LEFT].m_name = "attack2_left";
	m_animArray[ATTACK2_RIGHT].m_name = "attack2_right";

	m_animArray[ATTACK2_UP].m_name = "attack2_up";
	m_animArray[ATTACK2_DOWN].m_name = "attack2_down";

	m_animArray[ATTACK2_UP_LEFT].m_name = "attack2_up_left";
	m_animArray[ATTACK2_DOWN_LEFT].m_name = "attack2_down_left";

	m_animArray[ATTACK2_UP_RIGHT].m_name = "attack2_up_right";
	m_animArray[ATTACK2_DOWN_RIGHT].m_name = "attack2_down_right";


	m_animArray[SPECIAL1_LEFT].m_name = "special1_left";
	m_animArray[SPECIAL1_RIGHT].m_name = "special1_right";
	m_animArray[SPECIAL1_UP].m_name = "special1_up";
	m_animArray[SPECIAL1_DOWN].m_name = "special1_down";

	m_animArray[SPECIAL1_UP_LEFT].m_name = "special1_up_left";
	m_animArray[SPECIAL1_DOWN_LEFT].m_name = "special1_down_left";

	m_animArray[SPECIAL1_UP_RIGHT].m_name = "special1_up_right";
	m_animArray[SPECIAL1_DOWN_RIGHT].m_name = "special1_down_right";

	m_animArray[TURN_LEFT].m_name = "turn_left";
	m_animArray[TURN_RIGHT].m_name = "turn_right";
	m_animArray[TURN_UP].m_name = "turn_up";
	m_animArray[TURN_DOWN].m_name = "turn_down";

	m_animArray[TURN_UP_LEFT].m_name = "turn_up_left";
	m_animArray[TURN_DOWN_LEFT].m_name = "turn_down_left";

	m_animArray[TURN_UP_RIGHT].m_name = "turn_up_right";
	m_animArray[TURN_DOWN_RIGHT].m_name = "turn_down_right";


	m_animArray[JUMP_LEFT].m_name = "jump_left";
	m_animArray[JUMP_RIGHT].m_name = "jump_right";
	m_animArray[JUMP_UP].m_name = "jump_up";
	m_animArray[JUMP_DOWN].m_name = "jump_down";

	m_animArray[JUMP_UP_LEFT].m_name = "jump_up_left";
	m_animArray[JUMP_DOWN_LEFT].m_name = "jump_down_left";

	m_animArray[JUMP_UP_RIGHT].m_name = "jump_up_right";
	m_animArray[JUMP_DOWN_RIGHT].m_name = "jump_down_right";

}


VisualProfile::~VisualProfile()
{
}


bool VisualProfile::IsActive(int stateID)
{
	return m_animArray[stateID].m_pSprite != NULL;
}

vector<string> VisualProfile::GetListOfActiveAnims()
{
	vector<string> anims;

	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (IsActive(i))
		{
			//bingo
			anims.push_back(m_animArray[i].m_name);			
		}
	}
	return anims;
}

int VisualProfile::GetFirstValidAnimIndex()
{
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (IsActive(i))
		{
			//bingo
			return i;		
		}
	}

	return 0;
}

CL_Sprite * VisualProfile::GetSpriteByAnimID(int animID)
{
	if (animID < 0 || animID >= int(m_animArray.size()))
	{
		throw CL_Error("Bad anim id:" + CL_String::from_int(animID)+" in visual profile " + GetName());
	} else
	{

		if (!IsActive(animID))
		{
			
			int emergencyID = IDLE_LEFT;
			
			if (!IsActive(emergencyID))
			{
				emergencyID = GetFirstValidAnimIndex();
			}
			
			if (IsActive(emergencyID))
			{
				throw ("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID) + ".\n\nUnfortunately, no other animations in it's xml are found to use instead.");
				return NULL;
			} else
			{
				LogError( ("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID)).c_str() );
			}
			return m_animArray[emergencyID].m_pSprite;
		}
	}

	return m_animArray[animID].m_pSprite;
}

int VisualProfile::GetAnimID(int eState, int eFacing)
{
	int baseVisualState = (eState*FACING_COUNT);
	int animID = baseVisualState+eFacing;

	if (!IsActive(animID))
	{
		//if it's something small, let's fix it ourself
		
		switch (eFacing)
		{
		case FACING_UP_RIGHT:
		case FACING_UP_LEFT:
			animID = baseVisualState +FACING_UP;
			break;

		case FACING_DOWN_RIGHT:
		case FACING_DOWN_LEFT:
			animID = baseVisualState +FACING_DOWN;
			break;
		
		
		case FACING_LEFT:
			animID = baseVisualState +FACING_UP_LEFT;
			break;

		case FACING_RIGHT:
			animID = baseVisualState +FACING_DOWN_RIGHT;
			break;
		
		case FACING_UP:
			animID = baseVisualState +FACING_UP_RIGHT;
			break;

		case FACING_DOWN:
			animID = baseVisualState +FACING_DOWN_LEFT;
			break;
		}
	}

	if (!IsActive(animID) && IsActive( baseVisualState +FACING_LEFT) && IsActive( baseVisualState +FACING_RIGHT))
	{
		//well, the left and right are here, even though it looks like nothing else is.  Let's force the anim to be either left or right.

		switch (eFacing)
		{
		case FACING_UP_RIGHT:
		case FACING_DOWN_RIGHT:
		case FACING_UP:
			animID = baseVisualState +FACING_RIGHT;
			break;
		case FACING_UP_LEFT:
		case FACING_DOWN_LEFT:
		case FACING_DOWN:
			animID = baseVisualState +FACING_LEFT;
			break;
		}
		
	}

	if (!IsActive(animID))
	{
		//still can't locate it.
		animID = baseVisualState +FACING_LEFT;

		if (!IsActive(animID))
		{
			//we'll have to temporarily use a different anim set
			if (eState == VISUAL_STATE_WALK)
			{
				if (IsActive(RUN_LEFT) || IsActive(RUN_UP_LEFT))
				{
					//well, it looks like they have a run but not a walk.
					return GetAnimID(VISUAL_STATE_RUN, eFacing);
				}
			}
			
			if (eState == VISUAL_STATE_RUN) return GetAnimID(VISUAL_STATE_WALK, eFacing);
			if (eState == VISUAL_STATE_PAIN) return GetAnimID(VISUAL_STATE_IDLE, eFacing);
			if (eState == VISUAL_STATE_ATTACK2) return GetAnimID(VISUAL_STATE_ATTACK1, eFacing);
			
			if (eState != VISUAL_STATE_IDLE) //avoid recursion freeze
			{
				return GetAnimID(VISUAL_STATE_IDLE, eFacing);
			}
		}

	}

	if (!IsActive(animID))
	{
		int emergencyID = IDLE_LEFT;

		if (!IsActive(emergencyID))
		{
			emergencyID = GetFirstValidAnimIndex();
		}

		if (!IsActive(emergencyID))
		{
			LogMsg( ("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID)+" (" + m_animArray[animID].m_name+")").c_str() );	
			return -1;
		}

		return emergencyID;
	}

	return animID;
}

CL_Sprite * VisualProfile::GetSprite(int eState, int eFacing)
{
	int animID = GetAnimID(eState, eFacing);
	if (animID == -1) return NULL;

	if (!IsActive(animID))
	{
		animID = GetFirstValidAnimIndex();
	};

	return m_animArray[animID].m_pSprite;
}

int VisualProfile::StateNameToAnimID(const string & stState)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (stState == m_animArray[i].m_name)
		{
			//bingo
			return i;
		}
	}
	LogError("Unknown anim type: %s.  Keep in mind they are case sensitive.", stState.c_str());
	return -1;
}


int VisualProfile::SpriteNameToAnimID(const string & stState, bool bShowErrors)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (stState == m_animArray[i].m_spriteName)
		{
			//bingo
			return i;
		}
	}
	if (bShowErrors)
	{
		LogError("Unknown sprite name: %s.  Keep in mind they are case sensitive.", stState.c_str());
	}
	return -1;
}

int VisualProfile::SpriteNameToAnimID(CL_Sprite *pSprite)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (pSprite == m_animArray[i].m_pSprite)
		{
			//bingo
			return i;
		}
	}

	LogError("Can't find anim by sprite address");
	return -1;
}

int VisualProfile::TextToAnimIDCreatedIfNeeded(const string & stState)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (stState == m_animArray[i].m_name)
		{
			//bingo
			return i;
		}
	}

	//add it
	int id = m_animArray.size();
	ProfileAnim newProfile;
	newProfile.m_name = stState;

	m_animArray.push_back(newProfile);
	m_animArray[id].m_name = stState;
	return id;
}

void VisualProfile::AddAnimInfo(CL_DomElement &node)
{
	string stState = node.get_attribute("state");
	int animID = TextToAnimIDCreatedIfNeeded(stState);
	string stSpriteName = node.get_attribute("spritename");
	bool mirrorx = CL_String::to_bool(node.get_attribute("mirrorx"));
	bool mirrory = CL_String::to_bool(node.get_attribute("mirrory"));

	if (IsActive(animID))
	{
		LogError("Error, state %s already has a sprite attached to it in profile %s.", stState.c_str(), GetName().c_str());
	}

	try
	{
		m_animArray[animID].m_pSprite = new CL_Sprite(stSpriteName, m_pParent->m_pResourceManager);
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
		//clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);
	} catch(CL_Error error)
	{
		char st[4096];
		sprintf(st, "Error with putting anim %s as anim state %s while building profile %s.  Make sure you spelled it right and it's in the xml.\n(%s)",
			stSpriteName.c_str(), stState.c_str(), GetName().c_str(), error.message.c_str());
		SAFE_DELETE(m_animArray[animID].m_pSprite);
		
		ShowMessage("XML Anim loading error", st, true);
		return;
	}

	if (m_animArray[animID].m_pSprite->get_frame_count() == 0)
	{
		//something is wrong
		return;
	}
	m_animArray[animID].m_spriteName = stSpriteName;
	
	CL_Origin o;
	int x,y;

	if (mirrory)
	{
		m_animArray[animID].m_pSprite->set_angle_pitch(-180);
	}

	if (mirrorx)
	{
		m_animArray[animID].m_pSprite->set_angle_yaw(-180);
	}

	m_animArray[animID].m_pSprite->get_alignment(o, x, y);

	if (node.has_attribute("x"))
	{
		x = CL_String::to_int(node.get_attribute("x"));
	}
	if (node.has_attribute("y"))
	{
		y = CL_String::to_int(node.get_attribute("y"));
	}

	/*
	if (o == origin_top_left)
	{
		x += m_animArray[animID].m_pSprite->get_width();
	}
	*/

	m_animArray[animID].m_pSprite->set_alignment(o, x, y);

	m_animArray[animID].m_name = stState;
}

void VisualProfile::UpdateDocumentSpriteFromAnim(CL_DomNode *node, ProfileAnim *anim)
{
	//LogMsg("Updating %s", node.get_attributes().get_named_item("name").get_node_value().c_str());
	CL_DomNode cur_node;

	for (
		cur_node = node->get_first_child();
		!cur_node.is_null();
	cur_node = cur_node.get_next_sibling())
	{
		if (!cur_node.is_element()) continue;

		CL_DomElement cur_element = cur_node.to_element();

		if (cur_element.get_tag_name() == "translation")
		{
			CL_Origin o;
			int x,y;
			
			if (!anim->m_pSprite)
			{
				continue;
			}
			anim->m_pSprite->get_alignment(o, x,y);
			
			cur_element.set_attribute("x", CL_String::from_int(x));
			cur_element.set_attribute("y", CL_String::from_int(y));
			cur_element.set_attribute("origin", OriginToString(o));

		}
	} 
}

void VisualProfile::UpdateProfileAnim(CL_DomElement &node)
{
	string stState = node.get_attribute("state");
	int animID = TextToAnimIDCreatedIfNeeded(stState);

	//get offset data from the anim itself and plug it in if needed

	if (!IsActive(animID))
	{
		LogError("Strange, state %s has no valid sprite attached to it in profile %s.", stState.c_str(), GetName().c_str());
		return;
	}
	//LogMsg("Saving offsets to state %s in profile %s.", stState.c_str(), GetName().c_str());

	CL_Origin o;
	int x,y;

	m_animArray[animID].m_pSprite->get_alignment(o, x, y);

	node.set_attribute("x", CL_String::from_int(x));
	node.set_attribute("y", CL_String::from_int(y));
	

}
void VisualProfile::UpdateProfile(CL_DomNode *pNode)
{

	CL_DomNode cur_node;

	for (
		cur_node = pNode->get_first_child();
		!cur_node.is_null();
	cur_node = cur_node.get_next_sibling())
	{
		if (!cur_node.is_element()) continue;
		CL_DomElement cur_element = cur_node.to_element();
		string type = cur_element.get_tag_name();

		if (type == "anim")
		{
			UpdateProfileAnim(cur_element);
		} else
		{
			LogMsg("Warning: Don't know what %s is in the %s profile xml", type.c_str(), m_name.c_str());
		}
	}
}
void VisualProfile::UpdateSprite(CL_DomNode *pNode)
{
	CL_DomElement cur_element = pNode->to_element();

	string animName = cur_element.get_attribute("name", "");

	if (!animName.empty())
	{
		int id = SpriteNameToAnimID(animName);
		if (id != -1)
		{
			UpdateDocumentSpriteFromAnim(pNode, &m_animArray[id]);

		}
	}
}



void VisualProfile::UpdateToDocument(CL_DomDocument &document)
{
	
	CL_DomElement domRoot = document.get_document_element();
	CL_DomNodeList domList;
	domList = domRoot.get_elements_by_tag_name("profile");
	for (int c=0; c < domList.get_length(); c++)
	{
		string name = domList.item(c).get_attributes().get_named_item("name").get_node_value().c_str();

		if (name == GetName())
		{
			//bingo, this is the profile we want to edit
			UpdateProfile(&domList.item(c));

		}
	}


	//also update sprite information

	domList = domRoot.get_child_nodes();
	for (int c=0; c < domList.get_length(); c++)
	{
		
		if (domList.item(c).get_node_name() == "sprite")
		{
			//update this sprite info?
			UpdateSprite(&domList.item(c));
		}
	}
	
}

bool VisualProfile::Init(VisualResource *pVisualResource, const string &profileName)
{
	m_pParent = pVisualResource;
	m_name = profileName;

	CL_Resource resource = m_pParent->m_pResourceManager->get_resource(profileName);

	CL_DomNode cur_node;
	bool bFoundAnim = false;
	for (
		cur_node = resource.get_element().get_first_child();
		!cur_node.is_null();
	cur_node = cur_node.get_next_sibling())
	{
		if (!cur_node.is_element()) continue;
		CL_DomElement cur_element = cur_node.to_element();
		string type = cur_element.get_tag_name();

		if (type == "anim")
		{
			bFoundAnim = true;
			AddAnimInfo(cur_element);
		} else
		{
			LogMsg("Warning: Don't know what %s is in the %s profile xml", type.c_str(), profileName.c_str());
		}
	}

if (!bFoundAnim)
{
	LogError("Error in %s - You can't have a 'profile' and 'sprite' with the same name of %s.  Rename one of them.",
		m_pParent->GetFileName().c_str(),  m_name.c_str());
}

	return true; //success
}

//stuff for the lua docs

/*
Section: About Visual Profiles

About Visual Profiles:

A visual profile is an .xml text file that contains data explaining what an <Entity> should look like.

In general, it does not affect the actual game logic at all, you can switch out visual profiles later after prototyping your game with simple tilepic style images or other visual profiles.

(the only exception to the above is when *OnIdleLoop()* or *OnWalkLoop()* script functions are used to detect when an animation is looping and base game logic on that.)

Here is a very small but valid .xml visual profile called car.xml::

(code)
<resources>
	
	//first define the visual profile
	<profile name="car">
		<anim state="idle_left" spritename="red_car" mirrorx="no"/>
	</profile>

	//now define any animations the visual profile uses, in this case, red_car

	<sprite name="red_car">
		<image fileseq="myRedCar_.png" leading_zeroes="3"/>
		<translation origin="center"/>
		<animation pingpong="no" loop="yes" speed="150"/>
	</sprite>
</resources>
(end)

Now, in an <Entity>'s script, you would add this to its OnInit()::

(code)
function OnInit() //run upon initialization
	this:SetVisualProfile("car.xml", "red_car");
end
(end)

Easy, huh?  This would play a single animation of the 'car' for the <Entity>'s visual at all times.

The file it would try to load is *myRedCar_0000.png*,  *myRedCar_0001.png* and so on until it fails to find the next file in the sequence.

Note about loading a file sequence::

If myRedCar_0000.png isn' found, it will also try to start at myRedCar_0001.png.

Instead of loading a file sequence, you can also individually specify each frame you want like this::

(code)
<sprite name="red_car">
	<image file="carCrap.png"/>
	<image file="carCrapPart2.tga"/>
	<image file="AnotherFrame.jpg"/>
	<image file="TheLastFrame.bmp"/>

	<translation origin="center"/>
	<animation pingpong="no" loop="yes" speed="150"/>
</sprite>
(end)

Note about colorkey::

If we want an image or file sequence to automatically treat a specific color as transparent, you can add the "transparent_color" parameter.
(code)
//make white transparent
<image fileseq="myRedCar_.bmp" leading_zeroes="3" transparent_color="255,255,255" />
(end)

Alternatively, you may just use images that already have alpha information in them.

Visual State Hinting:

In the above example, *idle_left* is not a random animation name chosen, it is one of many pre-defined names that the engine will utilitize when deciding what image to display in the proper context.

So if your player is dying while facing right, the engine will check for *die_right*.  If that isn't found, it will check for *die_left*, if that fails, it will probably choose to use *idle_right*, and so on.

You can override and use additional custom named animations as well, for instance, the crayon.xml in the TreeWorld example uses a custom animation called *climb*.

A myriad of options for loading your sprite:

You can get info on most of the other settings as well as a lot of optional settings we haven't covered yet such as loading from a grid, setting animation timing of a specific frame, initial scale and alpha, rotational orientation and offset and more by checking out <http://www.clanlib.org/docs/clanlib-0.8.0/Overview/sprites_resources.html>.

Section: Related Constants

Group: Idle State Anims
Anim's with these names are preferred by the engine when in the <Idle> state.

constant: idle_left
If you want a single frame only for your entity, use only this state.  It's the final fallback if everything else fails.

constant: idle_right

constant: idle_up

constant: idle_down

constant: idle_up_left

constant: idle_down_left

constant: idle_up_right

constant: idle_down_right

Group: Walk State Anims
Anim's with these names are preferred by the engine when in the <Walk> state.

constant: walk_left

constant: walk_right

constant: walk_up

constant: walk_down

constant: walk_up_left

constant: walk_down_left

constant: walk_up_right

constant: walk_down_right


Group: Run State Anims
Anim's with these names are preferred by the engine when in the <Run> state. If not found, <Walk> anims will be used.

constant: run_left

constant: run_right

constant: run_up

constant: run_down

constant: run_up_left

constant: run_down_left

constant: run_up_right

constant: run_down_right

Group: Pain State Anims
Anim's with these names are preferred by the engine when in the <Pain> state.

constant: pain_left

constant: pain_right

constant: pain_up

constant: pain_down

constant: pain_up_left

constant: pain_down_left

constant: pain_up_right

constant: pain_down_right


Group: Die State Anims
Anim's with these names are preferred by the engine when in the <Die> state.

constant: die_left

constant: die_right

constant: die_up

constant: die_down

constant: die_up_left

constant: die_down_left

constant: die_up_right

constant: die_down_right

Group: Attack1 State Anims
Anim's with these names are preferred by the engine when in the <Attack> state.

constant: attack1_left

constant: attack1_right

constant: attack1_up

constant: attack1_down

constant: attack1_up_left

constant: attack1_down_left

constant: attack1_up_right

constant: attack1_down_right


Group: Special1 State Anims
Anim's with these names are preferred by the engine when in the <Pain> state.

constant: special1_left

constant: special1_right

constant: special1_up

constant: special1_down

constant: special1_up_left

constant: special1_down_left

constant: special1_up_right

constant: special1_down_right
*/

