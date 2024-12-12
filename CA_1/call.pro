QT += quick multimedia

QMAKE_LFLAGS += -fuse-ld=lld
CXXFKAGS += -fstack-protector

PATH_TO_SIO = "C:\Users\reyha\Downloads\supplementaries_CN_1 (2)\deps\socket.io-client-cpp"

SOURCES += \
    main.cpp \
    webrtc.cpp

HEADERS += \
    webrtc.h

RESOURCES += \
    resources.qrc

DEFINES += BOOST_DATE_TIME_NO_LIB
DEFINES += BOOST_REGEX_NO_LIB
DEFINES += ASIO_STANDALONE
DEFINES += _WEBSOCKETPP_CPP11_STL_
DEFINES += _WEBSOCKETPP_CPP11_FUNCTIONAL_

INCLUDEPATH += C:\Users\reyha\Downloads\libdatachannel\include \
                C:\Users\reyha\Downloads\opus\include

HEADERS += \
         $$PWD/SocketIO/sio_client.h \
         $$PWD/SocketIO/sio_message.h \
         $$PWD/SocketIO/sio_socket.h \
         $$PWD/SocketIO/internal/sio_client_impl.h \
         $$PWD/SocketIO/internal/sio_packet.h

SOURCES += \
         $$PWD/SocketIO/sio_client.cpp \
         $$PWD/SocketIO/sio_socket.cpp \
         $$PWD/SocketIO/internal/sio_client_impl.cpp \
         $$PWD/SocketIO/internal/sio_packet.cpp

INCLUDEPATH += $$PATH_TO_SIO/lib/websocketpp
INCLUDEPATH += $$PATH_TO_SIO/lib/asio/asio/include
INCLUDEPATH += $$PATH_TO_SIO/lib/rapidjson/include


LIBS += -LC:\Users\reyha\Downloads\opus\Windows\Mingw64 -lopus
LIBS += -LC:\Users\reyha\Downloads\libdatachannel\Windows\Mingw64\ -ldatachannel.dll
LIBS += -lssp
LIBS += -lws2_32
LIBS += -LC:\Qt\Tools\OpenSSLv3\Win_x64\bin -lcrypto-3-x64 -lssl-3-x64
