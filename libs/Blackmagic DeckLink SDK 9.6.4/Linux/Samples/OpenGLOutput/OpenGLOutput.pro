TEMPLATE  	= app
LANGUAGE  	= C++
CONFIG		+= qt opengl
QT			+= opengl
INCLUDEPATH =	../../include 

HEADERS 	=	OpenGLOutput.h \
    CDeckLinkGLWidget.h \
    BMDOpenGLOutput.h \
    GLScene.h \
    GLExtensions.h
SOURCES 	= 	main.cpp \
            	../../include/DeckLinkAPIDispatch.cpp \
            	OpenGLOutput.cpp \
    CDeckLinkGLWidget.cpp \
    BMDOpenGLOutput.cpp \
    GLScene.cpp \
    GLExtensions.cpp

FORMS 		= 	OpenGLOutput.ui
