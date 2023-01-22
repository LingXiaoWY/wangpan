TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/Mysql.cpp \
    src/TcpKernel.cpp \
    src/Thread_pool.cpp \
    src/block_epoll_net.cpp \
    src/clogic.cpp \
    src/err_str.cpp \
    src/main.cpp

HEADERS += \
    include/Mysql.h \
    include/TcpKernel.h \
    include/Thread_pool.h \
    include/block_epoll_net.h \
    include/clogic.h \
    include/err_str.h \
    include/packdef.h


INCLUDEPATH += ./include

LIBS += -lpthread -lmysqlclient
