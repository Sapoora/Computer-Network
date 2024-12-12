#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QIODevice>
#include <QAudioSource>
#include <opus.h>

class AudioInput : public QIODevice {
	Q_OBJECT

public:
	explicit AudioInput(QObject *parent = nullptr);
	~AudioInput() override;

	qint64 writeData(const char *data, qint64 len) override;
	Q_INVOKABLE void startRecording();
	Q_INVOKABLE void stopRecording();

Q_SIGNALS:
	void voicePacketReady(const QByteArray& data);

protected:
	qint64 readData(char *data, qint64 len) override;

private:
	QAudioSource *audioSource;
	OpusEncoder *encoder;
};

#endif // AUDIOINPUT_H
