var Std = require('std');
var Tcp = require('tcp');
var Utils = require('utils');
var console = {
	log: function() {
		Std.print.apply(undefined, arguments);
		Std.print("\n");
		Std.fflush(Std.stdout);
	},
	debug: function() {;
		Std.print("[" + getTime() + "] ");
		this.log.apply(this, arguments);
	}
}

var validRequestMethods = ["GET"];
var validRequestProtocols = ["HTTP/1.0", "HTTP/1.1"];

function HttpServer(handler) {
	this.__handler = handler;
	this.__tcp = null;
}

HttpServer.prototype.listen = function(port) {
	if (this.__tcp) {
		throw new Error("the server already listens");
	}
	var handler = this.__handler;
	this.__tcp = Tcp.createServer(function(address, request, response) {
		console.debug("new connection from " + address.host + ":" + address.port);
		
		request.addEventListener('data', function() {
			var requestHeaderRegex = /(.+)*\r\n/g;
			var str = Utils.toString(request.read());
			/* header */
			var requestHeader = requestHeaderRegex.exec(str);
			if (!requestHeader || !requestHeader[1]) {
				response.close(); /* close it */
				console.debug("malicious request, ignoring");
				return;
			}
			requestHeader = requestHeader[1];
			var requestHeaderFrags = requestHeader.split(" ");
			if (requestHeaderFrags.length !== 3) {
				response.close();
				console.debug("malicious request, ignoring");
				return;
			}
			var requestMethod = requestHeaderFrags[0];
			var requestUri = requestHeaderFrags[1];
			var requestProtocol = requestHeaderFrags[2];
			if (validRequestMethods.indexOf(requestMethod) < 0) {
				response.close();
				console.debug("malicious request, ignoring");
				return;
			}
			if (validRequestProtocols.indexOf(requestProtocol) < 0) {
				response.close();
				console.debug("malicious request, ignoring");
				return;
			}
			var requestObject = {
				method: requestMethod,
				uri: requestUri,
				protocol: requestProtocol,
				headers: {},
				address: { 
					host:   address.host, 
					port:   address.port, 
					family: address.family
				},
			};
			/* headers */
			var requestHttpHeader;
			var requestHttpHeaderRegex = /([^:]+):\s+(.+)/;
			while ((requestHttpHeader = requestHeaderRegex.exec(str)) && requestHttpHeader[1]) {
				requestHttpHeader = requestHttpHeader[1];
				var requestHttpHeaderFrags = requestHttpHeader.match(requestHttpHeaderRegex);
				if (!requestHttpHeaderFrags || !requestHttpHeaderFrags[1] || !requestHttpHeaderFrags[2]) {
					console.debug("malicious header, ignoring");
					continue;
				}
				requestObject.headers[requestHttpHeaderFrags[1]] = requestHttpHeaderFrags[2];
			}
			var responseBody = [];
			/* prepare response object */
			var responseObject = {
				code: 200,
				headers: {},
				write: function(chunk) {
					if ((typeof chunk !== "string") && !(chunk instanceof ArrayBuffer)) {
						throw new TypeError("Expected string or ArrayBuffer");
					}
					responseBody.push(chunk);
				}
			};
			/* request handler */
			handler.call(undefined, requestObject, responseObject);
			/* set defaults */
			if (!('Content-Length' in responseObject.headers)) {
				var responseBodyLength = responseBody.reduce(function(acc, el) {
					return el.byteLength || el.length;
				}, 0);
				responseObject.headers['Content-Length'] = responseBodyLength;
			}
			if (!('Connection' in responseObject.headers)) {
				responseObject.headers['Connection'] = 'keep-alive';
				responseObject.headers['Keep-Alive'] = 'timeout=300';
			}
			/* write response header */
			response.push("HTTP/1.1 " + responseObject.code + " OK\r\n");
			/* wrire response http headers */
			for (var key in responseObject.headers) {
				response.push(key + ": " + responseObject.headers[key] + "\r\n");
			}
			/* write divider */
			response.push("\r\n");
			/* write data */
			for (var i = 0; i < responseBody.length; ++i) {
				response.push(responseBody[i]);
			}
			/* done, send data */
			response.end();
			/* keep alive? */
			if ('Connection' in requestObject.headers) {
				if (requestObject.headers['Connection'] !== 'keep-alive') {
					response.close();
				}
				/* XXX: implement timeout */
			} else {
				response.close();
			}
		});

		request.addEventListener('close', function() {
			console.debug("connection closed " + address.host + ":" + address.port);
		});
	});
	this.__tcp.listen(port);
};

HttpServer.prototype.close = function() {
	if (this.__tcp) {
		this.__tcp.close();
		this.__tcp = null;
	}
};

module.exports = {
	createServer: function(handler) {
		if (typeof handler !== 'function') {
			throw new TypeError("expected handler to be a function");
		}
		return new HttpServer(handler);
	}
};

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
