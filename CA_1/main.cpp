#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <AudioInput.h>
#include <AudioOutput.h>
#include <webrtc.h>

int main(int argc, char * argv[]){
	QGuiApplication app(argc, argv);

	qmlRegisterType<AudioInput>("Audio", 1, 0, "AudioInput");
	qmlRegisterType<AudioOutput>("Audio", 1, 0, "AudioOutput");
	qmlRegisterType<WebRTC>("WebRTC", 1, 0, "RTC");

	QQmlApplicationEngine engine;

	QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] () {
		QCoreApplication::exit(-1);
	}, Qt::ConnectionType::QueuedConnection);

	engine.load("qrc:/Main.qml");

	return app.exec();
}

