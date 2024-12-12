import QtQuick 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import Audio 1.0
import WebRTC 1.0
import QtWebSockets 1.0

Window {
    width: 280
    height: 520
    visible: true
    title: qsTr("CA1")

    WebSocket {
        id: websocket
        url: "ws://localhost:9192"

        onTextMessageReceived: (text) => {
            console.log("Received message:", text);
            let jsonMessage = JSON.parse(text);

            if (jsonMessage.type === "sdp") {
                // Handle SDP message, setting remote description
                rtc.setRemoteDescription(jsonMessage.peerId, jsonMessage.sdp);
            } else if (jsonMessage.type === "ice-candidate") {
                // Handle ICE candidates
                rtc.setRemoteCandidate(jsonMessage.peerId, jsonMessage.candidate, jsonMessage.sdpMid);
            }
        }
    }

    AudioInput {
        id: audioInput
        onVoicePacketReady: (data) => {
            rtc.sendTrack(data);
        }
    }

    AudioOutput {
        id: audioOutput

    }

    RTC {
        id: rtc
        onIncomingPacket: (peerId, data) => {
            audioOutput.addData(data);
            if (!audioOutput.started) audioOutput.startPlaying();
        }

        onConnectionEstablished: {
            console.log("Connection established.");
            if (rtc.isOfferer) audioInput.startRecording();
        }

        // This should initiate SDP exchange based on offerer role
        isOfferer: (peerId, sdp) => {
            console.log("Sending SDP to signaling server:", sdp);
            websocket.send(JSON.stringify({ type: "sdp", peerId: peerId, sdp: sdp }));
        }

        Component.onCompleted: {
            // Optional: Additional setup if needed
        }
    }

    Item {
        anchors.fill: parent
        ColumnLayout {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: textfield.top
                margins: 20
            }

            Label {
                text: "Ip: " + "172.16.142.176"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }
            Label {
                text: "IceCandidate: " + "172.16.142.176"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }
            Label {
                text: "CallerId: " + textfield.text
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }
        }

        TextField {
            id: textfield
            placeholderText: "Phone Number"
            anchors.bottom: callbtn.top
            anchors.bottomMargin: 10
            anchors.left: callbtn.left
            anchors.right: callbtn.right
            enabled: !callbtn.pushed
        }

        Button {
            id: callbtn
            property bool pushed: false
            height: 47
            text: "Call"
            Material.background: "green"
            Material.foreground: "white"
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: 20
            }

            onClicked: {
                pushed = !pushed;

                if (pushed) {
                    Material.background = "red";
                    text = "End Call";

                    if (textfield.text === "o") {
                        // If this client is the offerer
                        rtc.init("offerer", true);
                        rtc.addPeer("answerer");
                    } else {
                        // If this client is the answerer
                        rtc.init("answerer", false);
                        rtc.addPeer("offerer");
                    }
                } else {
                    // End call logic
                    Material.background = "green";
                    text = "Call";
                    textfield.clear();
                    audioInput.stopRecording();
                    audioOutput.stopPlaying();
                    rtc.closeConnection();  // Add method to close connection gracefully
                }
            }
        }
    }
}
