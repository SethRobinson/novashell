INCLUDES = -I$(top_srcdir)/../../SharedLib -I$(top_srcdir)/../../SharedLib/lua 	-I$(top_srcdir)/../../SharedLib/lua/src \
-I$(top_srcdir)/../../SharedLib/luabind -I$(top_srcdir)/../../SharedLib/misc 	-I$(top_srcdir)/source \
-I$(top_srcdir)/source/AI -I/usr/local/include/ClanLib-1.0 $(all_includes)

METASOURCES = AUTO
noinst_LIBRARIES = libAI.a
libAI_a_SOURCES = AStarHeuristicPolicies.h Goal_Delay.cpp Goal_Delay.h \
		Goal_Evaluator.h Goal_FollowPath.cpp Goal.h Goal_MoveToMapAndPos.cpp \
		Goal_MoveToMapAndPos.h Goal_MoveToPosition.cpp Goal_MoveToPosition.h Goal_NegotiateDoor.cpp \
		Goal_NegotiateDoor.h Goal_RunScript.cpp Goal_RunScript.h Goal_SeekToPosition.cpp \
		Goal_SeekToPosition.h Goal_Think.cpp Goal_Think.h Goal_TraverseEdge.cpp Goal_TraverseEdge.h \
		HandyGraphFunctions.h invertedaabbox2d.h Makefile.am Makefile.in message_types.h \
		NavGraphManager.cpp NavGraphManager.h NodeTypeEnumerations.h ObjectEnumerations.h \
		PathManager.h PathPlanner.cpp PathPlanner.h PathTarget.h PriorityQueue.h \
		SearchTerminationPolicies.h SparseGraph.h TypeToString.h WatchManager.cpp WatchManager.h \
		WorldNavManager.cpp WorldNavManager.h Goal_Composite.h GraphEdgeTypes.h GraphNodeTypes.h \
		Goal_FollowPath.h PathEdge.h Goal_Approach.cpp Goal_Approach.h Goal_Say.cpp Goal_Say.h
