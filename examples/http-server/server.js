var Std = require('std');
var Http = require('http.js');
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

var port = Number(Std.getenv("PORT")) || 8080;

/* Static file service */

var servingDirPath = scriptArgs[0] || Std.getcwd();

try {
	var notFoundPage = loadFile(joinPaths(servingDirPath, "404.html"));
} catch (err) {
	var notFoundPage = "<h2>404 Not found</h2>";
}
try {
	var indexPage = loadFile(joinPaths(servingDirPath, "index.html"));
} catch (err) {
	var indexPage = "<body></body>";
}

var server = Http.createServer(function (request, response) {
	var uri = parseRequestUri(request.uri);
	var relPath = resolvePath("", uri.pathname);
	console.debug(request.method, relPath, request.protocol);
	if (!relPath || relPath === '/') {
		var page = indexPage;
		response.headers['Content-Type'] = "text/html";
	}
	else {
		try {
			response.headers['Content-Type'] = getMimeTypeForPath(relPath);
			var page = loadFile(joinPaths(servingDirPath, relPath));
		} catch (err) {
			response.headers['Content-Type'] = "text/html";
			response.code = 404;
			var page = notFoundPage;
		}
	}
	response.write(page);
});

server.listen(port);

console.debug('started listen server on port ', port);

function loadFile(path) {
	var fp = Std.fopen(path);
	Std.fseek(fp, 0, Std.SEEK_END);
	var size = Std.ftell(fp);
	Std.fseek(fp, 0, Std.SEEK_SET);
	var buffer = new ArrayBuffer(size);
	Std.fread(buffer, 1, size, fp);
	Std.fclose(fp);
	return buffer;
}

function joinPaths(path1, path2) {
	var resultedPath = path1.split(/[/\\]/).filter(Boolean);
	Array.prototype.push.apply(resultedPath, path2.split(/[/\\]/).filter(Boolean));
	return resultedPath.join("/");
}

function resolvePath(relto, path) {
	if (/^\w/.test(path)) return path;
	var resultedPath = relto.split(/[/\\]/).filter(Boolean);
	var pathComponents = path.split(/[/\\]/).filter(Boolean);
	var length = pathComponents.length;
	for (var i = 0; i < length; ++i) {
		if (pathComponents[i] == '..')
			resultedPath.pop();
		else if (pathComponents[i] != '.')
			resultedPath.push(pathComponents[i]);
	}
	return resultedPath.join("/");
}

function parseRequestUri(uri) {
	var rex = new RegExp([
	    '(/{0,1}[^?#]*)', // pathname
	    '(\\?[^#]*|)', // search
	    '(#.*|)$' // hash
	].join(''));
    var match = uri.match(rex);
    return match && {
        uri: uri,
        pathname: match[1],
        search: match[2],
        hash: match[3]
    }
}

function getMimeTypeForPath(path, def) {
	var ext = path.match(/\.(\w+)$/)[1];
	if (ext in mimeTypes) {
		return mimeTypes[ext];
	}
	return def || "application/octet-stream";
}

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

var mimeTypes = {
	"aac": "audio/aac",
	"abw": "application/x-abiword",
	"arc": "application/x-freearc",
	"avi": "video/x-msvideo",
	"azw": "application/vnd.amazon.ebook",
	"bin": "application/octet-stream",
	"bmp": "image/bmp",
	"bz": "application/x-bzip",
	"bz2": "application/x-bzip2",
	"csh": "application/x-csh",
	"css": "text/css",
	"csv": "text/csv",
	"doc": "application/msword",
	"docx": "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
	"eot": "application/vnd.ms-fontobject",
	"epub": "application/epub+zip",
	"gz": "application/gzip",
	"gif": "image/gif",
	"htm": "text/html",
	"html": "text/html",
	"ico": "image/vnd.microsoft.icon",
	"ics": "text/calendar",
	"jar": "application/java-archive",
	"jpeg": "image/jpeg",
	"jpg": "image/jpeg",
	"js": "text/javascript",
	"json": "application/json",
	"jsonld": "application/ld+json",
	"mid": "audio/midi",
	"midi": "audio/midi",
	"mjs": "text/javascript",
	"mp3": "audio/mpeg",
	"mpeg": "video/mpeg",
	"mpkg": "application/vnd.apple.installer+xml",
	"odp": "application/vnd.oasis.opendocument.presentation",
	"ods": "application/vnd.oasis.opendocument.spreadsheet",
	"odt": "application/vnd.oasis.opendocument.text",
	"oga": "audio/ogg",
	"ogv": "video/ogg",
	"ogx": "application/ogg",
	"opus": "audio/opus",
	"otf": "font/otf",
	"png": "image/png",
	"pdf": "application/pdf",
	"php": "application/php",
	"ppt": "application/vnd.ms-powerpoint",
	"pptx": "application/vnd.openxmlformats-officedocument.presentationml.presentation",
	"rar": "application/x-rar-compressed",
	"rtf": "application/rtf",
	"sh": "application/x-sh",
	"svg": "image/svg+xml",
	"swf": "application/x-shockwave-flash",
	"tar": "application/x-tar",
	"tif": "image/tiff",
	"tiff": "image/tiff",
	"ts": "video/mp2t",
	"ttf": "font/ttf",
	"txt": "text/plain",
	"vsd": "application/vnd.visio",
	"wav": "audio/wav",
	"weba": "audio/webm",
	"webm": "video/webm",
	"webp": "image/webp",
	"woff": "font/woff",
	"woff2": "font/woff2",
	"xhtml": "application/xhtml+xml",
	"xls": "application/vnd.ms-excel",
	"xlsx": "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
	"xml": "text/xml",
	"xul": "application/vnd.mozilla.xul+xml",
	"zip": "application/zip",
	"3gp": "video/3gpp",
	"3g2": "video/3gpp2",
	"7z": "application/x-7z-compressed",
};
