const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 9192 });

// Map to store clients by their chosen IDs
let clients = {};

wss.on('connection', (ws) => {
    let clientId = null;

    ws.on('message', (message) => {
        try {
            const data = JSON.parse(message);

            switch (data.type) {
                case 'register':
                    clientId = data.clientId;
                    clients[clientId] = ws;
                    console.log(`Client registered with ID: ${clientId}`);
                    break;

                case 'offer':
                    if (clients[data.targetId]) {
                        clients[data.targetId].send(JSON.stringify({
                            type: 'offer',
                            sdp: data.sdp,
                            senderId: clientId

                        }));
                          console.log(`Client offered  with ID: ${clientId}`);

                    }
                    break;

                case 'answer':
                    if (clients[data.targetId]) {
                        clients[data.targetId].send(JSON.stringify({
                            type: 'answer',
                            sdp: data.sdp,
                            senderId: clientId
                        }));
                    }
                      console.log(`Client Answered  with ID: ${clientId}`);

                    break;

                case 'candidate':
                    // Forward ICE candidate to the target peer
                    if (clients[data.targetId]) {
                        clients[data.targetId].send(JSON.stringify({
                            type: 'candidate',
                            candidate: data.candidate,
                            senderId: clientId
                        }));
                    }
                    break;

                default:
                    console.log('Unknown message type:', data.type);
            }
        } catch (error) {
            console.log('Failed to parse message:', error);
        }
    });

    ws.on('close', () => {
        console.log(`Client ${clientId} disconnected`);
        if (clientId) delete clients[clientId];
    });
});

console.log('WebSocket server is running on ws://localhost:9192');
