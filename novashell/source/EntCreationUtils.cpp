#include "AppPrecomp.h"
#include "EntCreationUtils.h"
#include "MovingEntity.h"
#include "Tile.h"
#include "EntWorldDialog.h"
#include "MaterialManager.h"
#include "EntChoiceDialog.h"

BaseGameEntity * CreateEntitySpecial(const string &EntName, const string &parms)
{

	if (CL_String::compare_nocase(EntName, "WorldChooseDialog"))
	{
		EntWorldDialog *pEnt = new EntWorldDialog();
		GetGameLogic()->GetMyEntityManager()->Add(pEnt);

		return pEnt;
	} else
		if (CL_String::compare_nocase(EntName, "ChoiceDialog"))
		{
			EntChoiceDialog *pEnt = new EntChoiceDialog(parms);
			GetGameLogic()->GetMyEntityManager()->Add(pEnt);
			return pEnt;
		}


	LogError("CreateEntitySpecial: Don't know what a %s is.", EntName.c_str());
	return 0;
}

MovingEntity * CreateEntity(Map *pMap, CL_Vector2 vecPos, string scriptFileName)
{
	MovingEntity *pEnt = new MovingEntity;
	TileEntity *pTile = new TileEntity;
	pTile->SetEntity(pEnt);
	pEnt->SetMainScriptFileName(scriptFileName);
	pEnt->SetPos(vecPos);
	pEnt->Init();
	
	if (!pMap) pMap = g_pMapManager->GetActiveMap();
	if (!pMap)
	{
		LogError("CreateEntity (%s at %s) failed, a null map was specified, and no active map found as backup!", scriptFileName.c_str(), VectorToString(&vecPos).c_str());
		delete pTile;
		return NULL;
	}
	pMap->AddTile(pTile);
	InitEntity(pEnt);

	
	return pEnt;
}

void InitEntity(MovingEntity *pEntity)
{

	bool bEditorHack = false;
	if (pEntity->IsPlaced() && GetGameLogic()->GetEditorActive() && GetGameLogic()->GetGamePaused())
	{
		//hack so we can run its init scripts now, so apples and such will start falling even if they aren't on the screen
		GetGameLogic()->SetGamePaused(false);
		//LogMsg("Ent %d is placed", pEntity->ID());
		bEditorHack = true;
	} else
	{
		//LogMsg("Ent %d is not placed", pEntity->ID());

	}
	
	pEntity->Update(0);
	pEntity->ApplyToPhysics(0);
	pEntity->PostUpdate(0);
	g_pMapManager->AddToEntityUpdateList(pEntity); //this will allow it to get a visibility notification or something
	if (bEditorHack)
	{
		GetGameLogic()->SetGamePaused(true);
	}
}

void ReInitEntity(MovingEntity *pEnt)
{
//we don't want to reinit things attached to the camera because generally those are GUI elements and get screwy
	float orientation = pEnt->GetRotation();
	CL_Vector2 facing = pEnt->GetVectorFacing();
	pEnt->Kill();
	pEnt->SetVectorFacing(facing);
	pEnt->Init();
	pEnt->SetRotation(orientation);
	InitEntity(pEnt);
}
void ReInitTileList(tile_list &t)
{
	
	tile_list::iterator itor = t.begin();

	Tile *pTile;

	while (itor != t.end())
	{
		pTile = (*itor);

		assert(pTile->GetType() < C_TILE_TYPE_COUNT);
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			ReInitEntity(((TileEntity*)pTile)->GetEntity());
		}


		itor++;
	}

}

void GetPointersToSimilarTilesOnMap(Map *pMap, tile_list &t, Tile *pTile)
{
	//get a list of all similar tiles, so we can update them when we're done if applicable
	pMap->GetMyMapCache()->AddTilesByRect(pMap->GetWorldRectInPixels(),
		&t, pMap->GetLayerManager().GetAllList(), false, false);
	//now we should remove any that aren't similar, or already on our selected list

	tile_list::iterator tItor = t.begin();

	while(tItor != t.end() )
	{
		assert((*tItor)->GetType() < C_TILE_TYPE_COUNT);
		if ( !TilesAreSimilar((*tItor), pTile))
		{
			//remove this one
			tItor = t.erase(tItor);
			continue;
		}
		tItor++;
	}

}
void AddShadowToParam1(CL_Surface_DrawParams1 &params1, Tile *pTile)
{
	CollisionData *pCol = pTile->GetCollisionData();
	if (!pCol) return;
	float density = 1 - 0.5;
	float bottomOffset = 0;
	float leftOffset = 0;

	float picSizeY = pTile->GetBoundsSize().y;

	line_list::iterator lineItor = pCol->GetLineList()->begin();

	PointList *pLine = NULL;

	for(;lineItor != pCol->GetLineList()->end(); lineItor++)
	{
		if (g_materialManager.GetMaterial((*lineItor).GetType())->GetType() != CMaterial::C_MATERIAL_TYPE_DUMMY)
		{
			//this looks fine
			pLine = &(*lineItor);
			break;
		}
	}

	if (pLine)
	{
		density = 0.8 - pLine->GetRect().get_height() / picSizeY;

/*	
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			CL_Rectf worldRect = pTile->GetWorldRect();
			bottomOffset = (worldRect.bottom - (pTile->GetPos().y + pLine->GetRect().get_height()/2))*0.9;
			leftOffset = (worldRect.left- (pTile->GetPos().x + pLine->GetRect().left))/2;
		} else
		{

		}
*/
		bottomOffset = pTile->GetWorldRect().bottom - (pTile->GetPos().y + pLine->GetRect().bottom );

		} 

	if (bottomOffset != 0)
	{
		//scale it

		bottomOffset *= GetCamera->GetScale().y ;

		//LogMsg("Bottom offset is %.2f", bottomOffset);
		//move the shadow closer to where the real bottom is
		params1.destY[0] -= bottomOffset;
		params1.destY[1] -= bottomOffset;
		params1.destY[2] -= bottomOffset;
		params1.destY[3] -= bottomOffset;

/*	
		leftOffset *= GetCamera->GetScale().x;

		//same thing with left to right
		params1.destX[0] -= leftOffset;
		params1.destX[1] -= leftOffset;
		params1.destX[2] -= leftOffset;
		params1.destX[3] -= leftOffset;
		*/

	}

	density = cl_max(0.1, density);
	density = cl_min(1, density);

	//LogMsg("Density is %.3f", density);

	
	float fWidthOffset = (params1.destY[3] - params1.destY[0])*density;
	float fHeightChange = (params1.destY[3] - params1.destY[0]) * density;

	/*
	static float angle = 0;
	angle+= GetApp()->GetDelta()/10;

	angle = fmod(double(angle), double(PI*2));

	float projectionWidth = fabs(params1.destX[0]- params1.destX[2]);
	float projectionHeight = fabs(params1.destY[0]- params1.destY[2]);
LogMsg("Projection Width: %.2f", projectionWidth);
	
	fWidthOffset = params1.destX[0] + (projectionWidth * cos(angle));
	fHeightChange = params1.destY[0] + (projectionHeight * sin(angle));
	//fHeightChange = cos(angle);
*/
	
	//skew top left
	params1.destX[0] -= fWidthOffset;
	params1.destX[1] -= fWidthOffset;

	if (density < 0.2f)
	{
		//remove skew, just move everything left

		params1.destX[2] -= (fWidthOffset * 0.7);
		params1.destX[3] -= (fWidthOffset * 0.7);

		params1.destY[2] -= fHeightChange * 0.3f;
		params1.destY[3] -= fHeightChange * 0.3f;
	}
	
	//squish things down a bit
	params1.destY[0] += fHeightChange;
	params1.destY[1] += fHeightChange;
	
}
