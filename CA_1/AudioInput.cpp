#include "AudioInput.h"

AudioInput::AudioInput(QObject *parent) : QIODevice(parent), audioSource(nullptr), encoder(nullptr) {

	int error;
	encoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &error);

	if (error != OPUS_OK) {
	}

	QAudioFormat format;
	format.setSampleRate(48000);
	format.setChannelCount(1);
	format.setSampleFormat(QAudioFormat::Int16);

	audioSource = new QAudioSource(format, this);
}

AudioInput::~AudioInput() {
	opus_encoder_destroy(encoder);
	delete audioSource;
}

qint64 AudioInput::writeData(const char *data, qint64 len) {

	unsigned char encodedData[4096];
	int encodedBytes = opus_encode(encoder, reinterpret_cast<const opus_int16*>(data), len / 2, encodedData, sizeof(encodedData));

	if (encodedBytes < 0) {

		return -1;
	}


	QByteArray data2;

	Q_EMIT voicePacketReady(data2);

	return len;
}

void AudioInput::startRecording() {
	audioSource->start(this);
}

void AudioInput::stopRecording() {
	audioSource->stop();
}

qint64 AudioInput::readData(char *data, qint64 len) {

	return 0;
}
