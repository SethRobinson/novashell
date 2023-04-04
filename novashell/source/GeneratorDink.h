#pragma once

#include "GeneratorInterface.h"

class Map;
class Screen;

struct dink_map_info
{
	char name[20];
	int loc[769];
	int music[769];
	int indoor[769];
	int v[40];
	char s[80];
	char buffer[2000];

};

struct dink_tile
{
	int num, property, althard, more2;
	unsigned char  more3,more4;

	int buff[15];
};

struct dink_sprite_placement
{
	int x,y,seq,frame, type,size;
	bool active;
	int rotation, special,brain;


	char script[13];
	char hit[13];
	char die[13];
	char talk[13];
	int speed, base_walk,base_idle,base_attack,base_hit,timer,que;
	int hard;
	CL_Rect alt;
	int prop;
	int warp_map;
	int warp_x;
	int warp_y;
	int parm_seq;

	int base_die, gold, hitpoints, strength, defense,exp, sound, vision, nohit, touch_damage;
	int buff[5];
};

struct dink_small_map
{
	char name[20];
	dink_tile t[97];
	int v[40];
	char s[80];
	dink_sprite_placement sprite[101];

	char script[13];
	char random[13];
	char load[13];
	char buffer[1000];
};

class GeneratorDink: public GeneratorInterface
{
public:

	GeneratorDink();

	virtual void GenerateInit();
	virtual bool GenerateStep();

protected:

	void LoadDinkScreen(dink_small_map *pDinkScreen, int mapIndex);
	void ProcessDinkScreen(CL_Vector2 &vecOffset, dink_small_map &screenData);

	int m_mapIndex;
	dink_map_info m_mapInfo;
	CL_Vector2 m_vecMapUpperLeft;
};
