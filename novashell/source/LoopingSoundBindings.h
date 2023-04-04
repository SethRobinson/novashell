#ifndef LoopingSoundBindings_HEADER_INCLUDED // include guard
#define LoopingSoundBindings_HEADER_INCLUDED  // include guard


//helper for looping sounds
class LoopingSound
{
public:

	LoopingSound();
	LoopingSound(const string &file);

	virtual ~LoopingSound();

	void Init(const string &file);
	void Finalize();

	void Play(bool bOn);

private:

	string m_file;
	int m_handle;

};

void luabindLoopingSound(lua_State *pState);


#endif        