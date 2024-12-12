#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QIODevice>
#include <QMutex>
#include <QAudioSink>
#include <QAudioFormat>
#include <QQueue>
#include <QBuffer>
#include <opus.h>

class AudioOutput : public QObject {
	Q_OBJECT

public:
	explicit AudioOutput(QObject *parent = nullptr);
	~AudioOutput() override;

	Q_INVOKABLE void addData(const QByteArray &data);
	void play();
	Q_INVOKABLE void startPlaying();
	//Q_INVOKABLE void stop();

	bool getStared() const;
	void setStared(bool newStared);
	void resetStared();
	void stopPlaying();


signals:
	void newPacket();

	void staredChanged();

private:
	QAudioSink *audioSink;
	QAudioFormat audioFormat;
	QQueue<QByteArray> packetQueue;
	QMutex mutex;
	OpusDecoder *decoder;
	QBuffer *audioBuffer;
	bool stared = false;
	QIODevice *device;


	Q_PROPERTY(bool stared READ getStared WRITE setStared RESET resetStared NOTIFY staredChanged FINAL)
};

#endif // AUDIOOUTPUT_H
