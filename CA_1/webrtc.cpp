#include "webrtc.h"
#include <QtEndian>
#include <QJsonDocument>



static_assert(true);

#pragma pack(push, 1)
struct RtpHeader {
    uint8_t first;
    uint8_t marker:1;
    uint8_t payloadType:7;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
};
#pragma pack(pop)


WebRTC::WebRTC(QObject *parent)
    : QObject{parent},
    m_audio("Audio")
{
    connect(this, &WebRTC::gatheringComplited, [this] (const QString &peerID) {
        if(!m_gatheringComplited) return;

        m_localDescription = descriptionToJson(m_peerConnections[peerID]->localDescription().value());
        Q_EMIT localDescriptionGenerated(peerID, m_localDescription);

        if (m_isOfferer)
            Q_EMIT this->offerIsReady(peerID, m_localDescription);
        else
            Q_EMIT this->answerIsReady(peerID, m_localDescription);
    });
}

WebRTC::~WebRTC()
{}


/**
 * ====================================================
 * ================= public methods ===================
 * ====================================================
 */

void WebRTC::init(const QString &id, bool isOfferer)
{
    // Initialize WebRTC using libdatachannel library

    m_isOfferer = isOfferer;

    // Create an instance of rtc::Configuration to Set up ICE configuration

    rtc::Configuration config;


    // Add a STUN server to help peers find their public IP addresses

    config.iceServers.push_back({"stun:stun.l.google.com:19302"});


	// Add a TURN server for relaying media if a direct connection can't be established

	rtc::IceServer turnServer("turn:your_turn_server_url");
	turnServer.username = "username";
	turnServer.password = "password";
	config.iceServers.push_back(turnServer);


    // Set up the audio stream configuration

    m_peerConnections[id] = std::make_shared<rtc::PeerConnection>(config);

}





void WebRTC::addPeer(const QString &peerId)
{
	rtc::Configuration config;
	config.iceServers.push_back({"stun:stun.l.google.com:19302"});

	auto newPeer = std::make_shared<rtc::PeerConnection>(config);

    newPeer->onLocalDescription([this,peerId](const rtc::Description &description) {
		m_localDescription = descriptionToJson(description);
	});

    // // Set up a callback for when the state of the peer connection changes
    newPeer->onStateChange([this, peerId](rtc::PeerConnection::State state) {
        // Handle different states like New, Connecting, Connected, Disconnected, etc.

		switch (state) {
		case rtc::PeerConnection::State::New:
			// Handle the "New" state if needed
			break;
		case rtc::PeerConnection::State::Connecting:
			// Handle connecting state
			break;
		case rtc::PeerConnection::State::Connected:
			// Connection established
			Q_EMIT connectionEstablished(peerId);
			break;
		case rtc::PeerConnection::State::Disconnected:
			// Handle disconnection
			Q_EMIT connectionDisconnected(peerId);
			break;
		case rtc::PeerConnection::State::Failed:
			// Handle connection failure
			Q_EMIT connectionFailed(peerId);
			break;
		case rtc::PeerConnection::State::Closed:
			// Handle closure of the connection
			break;
		}

	});



    // // Set up a callback for monitoring the gathering state
    newPeer->onGatheringStateChange([this, peerId](rtc::PeerConnection::GatheringState state) {
        // When the gathering is complete, emit the gatheringComplited signal

		if (state == rtc::PeerConnection::GatheringState::Complete) {
			m_gatheringComplited = true;
			Q_EMIT gatheringComplited(peerId);
		}

	});

    // // Set up a callback for handling incoming tracks
    newPeer->onTrack([peerId] (std::shared_ptr<rtc::Track> track) {
        qDebug() << "new track is available";
	});

    // Add an audio track to the peer connection

    m_peerConnections[peerId] = newPeer;

    addAudioTrack(peerId, "track01");
}




// Set the local description for the peer's connection


void WebRTC::generateOfferSDP(const QString &peerId)
{
    // Ensure the peer connection exists

}






// Generate an answer SDP for the peer
void WebRTC::generateAnswerSDP(const QString &peerId)
{
	// auto peerConnection = m_peerConnections.value(peerId);

	// if (!peerConnection) qDebug() << "error";


}





// Add an audio track to the peer connection
void WebRTC::addAudioTrack(const QString &peerId, const QString &trackName)
{

	if (m_debugingMode) iDebugRTC << "adding audio track to: " << peerId;

	m_audio.addSSRC(m_ssrc,trackName.toStdString(),
					QString("stream%1").arg(m_instanceCounter++).toStdString(),
					trackName.toStdString());


	auto track = m_peerConnections[peerId]->addTrack(m_audio);


	track->onOpen([this] () {

			 if (m_debugingMode) iDebugRTC << "track is open" ;

	});

	track->onAvailable([this] () {

		if (m_debugingMode) iDebugRTC << "track is available" ;

	});

	track->onError([this] (auto error) {

		if (m_debugingMode) iDebugRTC << "track is on Error" <<error.c_str();

	});

	track->onClosed([this] () {

		if (m_debugingMode) iDebugRTC << "track is closed" ;

	});

	track->onMessage([this, peerId](rtc::message_variant data) {

		auto packet = this->readVariant(data);
		constexpr int headerLength = sizeof(rtc::RtpHeader);
		packet.remove(0,headerLength);

		if(m_debugingMode) iDebugRTC << "track massage: " << packet.size() <<"bytes" ;

		Q_EMIT incomingPacket(peerId,packet);
	});



		   // Connect to the onFrame signal to receive audio frames
	track->onFrame([this](rtc::binary frame, rtc::FrameInfo info) {

		Q_UNUSED(frame);
		if(m_debugingMode)
			iDebugRTC << "track frame: "
					  << "frame info: " <<"payLoadType: " <<info.payloadType << "timestamp: " <<info.timestamp;
	});

	m_peerTracks[peerId] = track;
}



// Sends audio track data to the peer

void WebRTC::sendTrack(const QString &peerId, const QByteArray &buffer)
{

	if(m_debugingMode)
		iDebugRTC << "sending tack to: " << peerId
				  <<"track count:" << m_peerTracks.size() ;


	if(!m_peerTracks[peerId]->isOpen()) {
		iDebugRTC << "track is not open";
		return;
	}

	RtpHeader rtpHeader;
	rtpHeader.first = 0b10000000;
	rtpHeader.marker = 0;
	rtpHeader.payloadType = m_payloadType;
	rtpHeader.sequenceNumber = qToBigEndian(m_sequenceNumber++);
	rtpHeader.timestamp = qToBigEndian(getCurrentTimestamp());
	rtpHeader.ssrc = qToBigEndian(m_ssrc);
	QByteArray rtpPacket;
	rtpPacket.append(reinterpret_cast<const char*>(&rtpHeader),sizeof(rtc::RtpHeader));
	rtpPacket.append(buffer);

	try {
		bool result = m_peerTracks[peerId]->send(reinterpret_cast<const std::byte *>(rtpPacket.data()),rtpPacket.size());
		if(result) iDebugRTC<<"one packet sent successfully";
		else iDebugRTC << "packet sending failed";
	} catch (std::runtime_error &e) {
		iDebugRTC << "error sending track: what?:" << e.what();
	} catch (...) {
		iDebugRTC << "error sending track: unknown" ;
	}
}


/**
 * ====================================================
 * ================= public slots =====================
 * ====================================================
 */

// Set the remote SDP description for the peer that contains metadata about the media being transmitted
void WebRTC::setRemoteDescription(const QString &peerID, const QString &sdp)
{

	if(m_debugingMode)
		iDebugRTC << "peer: " << peerID
				  <<"remote description: "
				  <<"sdp: "<< sdp;

	m_remoteDescription = sdp.toUtf8();
	QJsonDocument doc = QJsonDocument::fromJson(sdp.toUtf8());

	rtc::Description description(doc["sdp"].toString().toStdString(),
								 doc["type"].toString().toStdString());


	m_peerConnections[peerID]->setRemoteDescription(description);



}

// Add remote ICE candidates to the peer connection
void WebRTC::setRemoteCandidate(const QString &peerID, const QString &candidate, const QString &sdpMid)
{
	if(m_debugingMode)
		iDebugRTC << "Remote Description: "
				  <<"Candidate:" << candidate
				  <<"sdpmid: " << sdpMid
				  <<"peer:" << peerID;

	m_peerConnections[peerID]->addRemoteCandidate(candidate.toStdString());

}


/*
 * ====================================================
 * ================= private methods ==================
 * ====================================================
 */

// Utility function to read the rtc::message_variant into a QByteArray
QByteArray WebRTC::readVariant(const rtc::message_variant &data)
{

	QByteArray packet(reinterpret_cast<const char*>(std::get<rtc::binary>(data).data()),std::get<rtc::binary>(data).size());
	return packet;

}

// Utility function to convert rtc::Description to JSON format
QString WebRTC::descriptionToJson(const rtc::Description &description)
{
	auto temp = QString("{\"type\": \"%1\", \"sdp\": \"%2\")");
	auto typeString = QString::fromStdString(description.typeString());
	auto sdp = QString::fromStdString(description);
	QJsonDocument doc = QJsonDocument::fromJson(temp.arg(typeString,sdp).toUtf8());
	return doc.toJson();
}

// Retrieves the current bit rate
int WebRTC::bitRate() const
{
	return m_bitRate;

}

// Set a new bit rate and emit the bitRateChanged signal
void WebRTC::setBitRate(int newBitRate)
{
	if(m_bitRate == newBitRate) return;
	m_bitRate = newBitRate;
	Q_EMIT bitRateChanged();

}

// Reset the bit rate to its default value
void WebRTC::resetBitRate()
{
	setBitRate({});

}

// Sets a new payload type and emit the payloadTypeChanged signal
void WebRTC::setPayloadType(int newPayloadType)
{
	if(m_payloadType == newPayloadType) return;
	m_payloadType = newPayloadType;
	Q_EMIT payloadTypeChanged();

}

// Resets the payload type to its default value
void WebRTC::resetPayloadType()
{
	setPayloadType({});

}

// Retrieve the current SSRC value
rtc::SSRC WebRTC::ssrc() const
{
	return m_ssrc;

}

// Set a new SSRC and emit the ssrcChanged signal
void WebRTC::setSsrc(rtc::SSRC newSsrc)
{
	if(m_ssrc == newSsrc) return;
	m_ssrc = newSsrc;
	Q_EMIT ssrcChanged();

}

// Reset the SSRC to its default value
void WebRTC::resetSsrc()
{
	setSsrc({});


}

// Retrieve the current payload type
int WebRTC::payloadType() const
{
    return m_payloadType;

}


/**
 * ====================================================
 * ================= getters setters ==================
 * ====================================================
 */

bool WebRTC::isOfferer() const
{
    return m_isOfferer;

}

void WebRTC::setIsOfferer(bool newIsOfferer)
{
	if (m_isOfferer != newIsOfferer) {
		m_isOfferer = newIsOfferer;
	}

}

void WebRTC::resetIsOfferer()
{
	setIsOfferer(false);

}


