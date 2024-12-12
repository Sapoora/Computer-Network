#include "AudioOutput.h"
#include <QMutexLocker>
#include <QDebug>

AudioOutput::AudioOutput(QObject *parent) : QObject(parent), audioSink(nullptr), decoder(nullptr) {
	audioFormat.setSampleRate(48000);
	audioFormat.setChannelCount(1);
	audioFormat.setSampleFormat(QAudioFormat::Int16);

	audioSink = new QAudioSink(audioFormat, this);

	int error;
	decoder = opus_decoder_create(48000, 1, &error);
	if (error != OPUS_OK) {
		qDebug() << "Failed to create Opus decoder!";
	}

	audioBuffer = new QBuffer(this);
	audioBuffer->open(QIODevice::ReadWrite);

	connect(this, &AudioOutput::newPacket, this, &AudioOutput::play);
}

AudioOutput::~AudioOutput() {
	if (decoder) {
		opus_decoder_destroy(decoder);
	}
	delete audioSink;
}

void AudioOutput::addData(const QByteArray &data) {
	QMutexLocker locker(&mutex);

	packetQueue.enqueue(data);

	emit newPacket();
}

void AudioOutput::play() {
	QMutexLocker locker(&mutex);
	if (!packetQueue.isEmpty()) {
		QByteArray packet = packetQueue.dequeue();

		opus_int16 decodedData[48000];
		int decodedBytes = opus_decode(decoder, reinterpret_cast<const unsigned char*>(packet.data()), packet.size(), decodedData, sizeof(decodedData) / sizeof(decodedData[0]), 0);

		if (decodedBytes > 0) {
			audioBuffer->write(reinterpret_cast<const char*>(decodedData), decodedBytes * sizeof(opus_int16));

			audioBuffer->seek(0);

			audioSink->start(audioBuffer);
		}
	}
}

void AudioOutput::startPlaying()
{
	device = audioSink->start();
	setStared(true);
}

bool AudioOutput::getStared() const
{
	return stared;
}

void AudioOutput::setStared(bool newStared)
{
	if (stared == newStared)
		return;
	stared = newStared;
	emit staredChanged();
}

void AudioOutput::resetStared()
{
	setStared({}); // TODO: Adapt to use your actual default value
}

void AudioOutput::stopPlaying() {
	if (audioSink) {  // Check if audio output is available
		audioSink->stop();
		// Additional cleanup if necessary
	}
}


