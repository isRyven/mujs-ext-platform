var console = {
	log: function() {
		Std.print.apply(undefined, arguments);
		Std.print("\n");
		Std.fflush(Std.stdout);
	}
}

function log(msg) {
	console.log("[" + getTime() + "]", msg);
}

var port = Number(Std.getenv("PORT")) || 8080;
var clients = {};
var uniqueId = 0;

function broadcastMessage(msg) {
	for (var k in clients) {
		var client = clients[k];
		client.response.push(msg);
		client.response.end();
	}
}

var server = Tcp.createServer(function(address, request, response) {
	var clientId = uniqueId++; 
	var clientIp = address.host + ":" + address.port;
	var clientName = "user" + clientId + "@" + clientIp;
	
	clients[clientId] = {
		address: clientIp,
		response: response,
		name: clientName
	};

	function logClient(msg) {
		log(clientName + " " + msg.replace(/\n$/, ""));
	}

	logClient("joined the server");
	broadcastMessage(clientName + " joined the server\n");

	request.addEventListener('data', function() {
		var str = Utils.toString(request.read());
		logClient(": " + str);
		var msg = clientName + ": " + str; 
		broadcastMessage(msg);
	});

	request.addEventListener('close', function() {
		delete clients[clientId];
		logClient("left the server");
		broadcastMessage(clientName + " left the server\n");
	});
});

server.listen(port);
console.log('started listen server on port ', port);

function getTime() {
	var d = new Date();
	var h = d.getHours();
	var m = d.getMinutes();
	var s = d.getSeconds();
	if (h < 10) h = '0' + h;
	if (m < 10) m = '0' + m;
	if (s < 10) s = '0' + s;
	return h + ":" + m + ":" + s;
}
