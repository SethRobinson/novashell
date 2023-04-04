
bin_PROGRAMS = novashell
novashell_SOURCES = AppPrecomp.cpp AppUtils.cpp AppUtils.h BaseGameEntity.cpp \
						BaseGameEntity.h Brain.cpp Brain.h Camera.cpp Camera.h CameraSetting.cpp CameraSetting.h \
						CollisionData.cpp CollisionData.h Console.cpp Console.h DataManager.cpp DataManager.h \
						EntChooseScreenMode.cpp EntChooseScreenMode.h EntCollisionEditor.cpp EntCollisionEditor.h \
						EntEditMode.cpp EntEditMode.h EntEditor.cpp EntEditor.h EntityBindings.cpp \
						EntityBindings.h EntityManager.cpp EntityManager.h GameLogic.cpp GameLogic.h \
						GeneratorDink.cpp GeneratorDink.h GeneratorInterface.h GeneratorSimple.cpp \
						GeneratorSimple.h HashedResource.cpp HashedResource.h HashedResourceManager.cpp \
						HashedResourceManager.h LayerManager.cpp LayerManager.h luabindBindings.cpp luabindBindings.h \
						main.cpp main.h MaterialManager.cpp MaterialManager.h Message.cpp Message.h \
						MessageManager.cpp MessageManager.h MiscClassBindings.cpp MiscClassBindings.h \
						MovingEntity.cpp movingentity.h MyEntityManager.cpp MyEntityManager.h PointList.cpp \
						PointList.h Screen.cpp screen.h ScriptManager.cpp ScriptManager.h TagManager.cpp \
						TagManager.h TextManager.cpp TextManager.h Tile.cpp TileEditOperation.cpp \
						TileEditOperation.h TileEntity.cpp TileEntity.h Tile.h VectorBindings.cpp VectorBindings.h \
						VisualProfile.cpp VisualProfile.h VisualProfileManager.cpp VisualProfileManager.h \
						VisualResource.cpp VisualResource.h Matrix.h BrainManager.cpp BrainManager.h \
						BrainShake.cpp BrainShake.h BrainColorFlash.cpp BrainFadeOutAndDelete.cpp State.cpp \
						StateDie.cpp StateFrozen.cpp Trigger.cpp BrainTopBase.cpp BrainTopBase.h \
						StateTopAttack.cpp StateTopAttack.h StateTopIdle.cpp StateTopIdle.h StateTopWalk.cpp \
						StateTopWalk.h BrainTopSeek.cpp EntVisualProfileEditor.cpp EntCreationUtils.cpp \
						EntWorldDialog.cpp EntChoiceDialog.cpp BrainColorFade.cpp BrainBob.cpp BrainFadeIn.cpp \
						GlobalScriptFunctionBindings.cpp BrainForceVisual.cpp BrainForceVisual.h ListBindings.cpp ListBindings.h \
						LoopingSoundBindings.cpp LoopingSoundBindings.h TileBindings.cpp TileBindings.h StatePain.cpp \
						EntMapCache.cpp EntMapCache.h InputManager.cpp InputManager.h MapChunk.cpp MapChunk.h \
						Map.cpp Map.h MapManager.cpp MapManager.h BrainScale.cpp \
				EffectManagerBindings.cpp EffectManagerBindings.h EffectManager.cpp EffectManager.h \
			DataEditor.cpp DataEditor.h ScriptCallbackManager.cpp ScriptCallbackManager.h \
	StateAttack2.cpp StateAttack2.h StateJump.cpp StateJump.h StateRun.cpp StateRun.h NovaZip.cpp \
	StateSpecial1.cpp StateSpecial1.h StateTurn.cpp StateTurn.h PhysicsManager.cpp DebugDraw.cpp DrawManager.cpp \
	BrainDoomWall.cpp BrainLife.cpp WorldPackager.cpp \
	$(top_srcdir)/../../SharedLib/misc/CL_VirtualFileManager.cpp $(top_srcdir)/../../SharedLib/misc/MiscUtils.cpp \
	$(top_srcdir)/../../SharedLib/misc/CL_SoundManager.cpp


# set the include path found by configure
INCLUDES = -I$(top_srcdir)/../../SharedLib -I$(top_srcdir)/../../SharedLib/lua 	-I$(top_srcdir)/../../SharedLib/lua/src \
-I$(top_srcdir)/../../SharedLib/luabind -I$(top_srcdir)/../../SharedLib/misc 	-I$(top_srcdir)/source \
-I$(top_srcdir)/source/AI -I/usr/local/include/ClanLib-1.0 $(all_includes)

# the library search path.


novashell_LDFLAGS = $(all_libraries) `pkg-config --cflags --libs clanApp-1.0 clanCore-1.0 clanDisplay-1.0 clanGL-1.0 clanVorbis-1.0 clanGUIStyleSilver-1.0`
novashell_LDADD =  \
	$(top_builddir)/source/GUIStyleBitmap/libGuiStyleBitmap.a $(top_builddir)/source/AI/libAI.a \
	$(top_builddir)/../../SharedLib/linearparticle/sources/libeffects.a \
	$(top_builddir)/../../SharedLib/luabind/src/libluabind.a \
	$(top_builddir)/../../SharedLib/lua/src/liblualib.a \
	$(top_builddir)/../../SharedLib/Box2D/Source/Gen/float/libbox2d.a


#/usr/local/lib/libclanVorbis.a /usr/local/lib/libclanSound.a /usr/local/lib/libclanGUIStyleSilver.a \
#	/usr/local/lib/libclanGUI.a /usr/local/lib/libclanGL.a /usr/local/lib/libclanDisplay.a \
#	/usr/local/lib/libclanApp.a /usr/local/lib/libclanCore.a /usr/local/lib/libclanSignals.a -lGLU \
#	/usr/lib/libvorbis.a /usr/lib/libpng12.a
#/usr/lib/libtiff.a /usr/lib/libjpeg.a

# -lGL -lXi /usr/lib/libogg.a -lpthread -lxml2 -lm \
#	-lbz2 -ldl -lz -lXext -lXt -lX11 -lSM \
#	-lasound -lICE -lXxf86vm -ldl

SUBDIRS = GUIStyleBitmap AI
