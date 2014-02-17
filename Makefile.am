ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= MinimalOgre.h Ball.h OgreMotionState.h Simulator.h

bin_PROGRAMS= OgreApp
OgreApp_CPPFLAGS= -I$(top_srcdir)
OgreApp_SOURCES= MinimalOgre.cpp
OgreApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS)
OgreApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS)

EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
