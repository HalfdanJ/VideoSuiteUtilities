TEMPLATE  	= app
LANGUAGE  	= C++
CONFIG		+= qt opengl
QT			+= opengl
INCLUDEPATH =	../../include 

HEADERS 	=	SignalGenerator.h
SOURCES 	= 	main.cpp \
            	../../include/DeckLinkAPIDispatch.cpp \
            	SignalGenerator.cpp

FORMS 		= 	SignalGenerator.ui

