function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
function noexcept(f, m) { try { f() } catch (e) { throw new Error(e || m); } }
/* assume string is always ASCII encoded */
function toStrBuffer(str) {
	var size = str.length;
	var ab = new ArrayBuffer(size);
	var ta = new Uint8Array(ab);
	for (var i = 0; i < size; ++i) {
		ta[i] = str[i].charCodeAt(0);
	}
	return ab;
}
/* assume buffer always contains ASCII encoded string buffer */
function toStrFromBuffer(ab) {
	var ta = new Uint8Array(ab);
	var str = String.fromCharCode.apply(String, ta);
	return str;
}
function toStrZFromBuffer(ab) {
	var ta = new Uint8Array(ab);
	var str = [];
	for (var i = 0; i < ta.length; ++i) {
		if (ta[i] === 0) {
			break;
		}
		str.push(String.fromCharCode(ta[i]));
	}
	return str.join("");
}

var phrase = "hello, world";

assert(std);
assert(std.fopen);
assert(std.fclose);
assert(std.fread);
assert(std.fwrite);
assert(std.fseek);
assert(std.ftell);
assert(std.feof);
assert(std.ferror);
assert(std.fflush);
assert(std.fgetc);
assert(std.fgets);
assert(std.fputc);
assert(std.fputs);
assert(std.remove);
assert(std.rename);
assert(std.rewind);
assert(std.tmpfile);
assert(std.getcwd);
assert(std.getenv);
assert(std.print);
assert(std.readFile);
assert(std.writeFile);
assert(std.exit);
assert(std.mkdir);
assert(std.readdir);
assert(std.stat);
assert(std.exists);
assert(std.rmtree);
assert(std.stdout);
assert(std.stdin);
assert(typeof std.SEEK_SET === "number");
assert(typeof std.SEEK_CUR === "number");
assert(typeof std.SEEK_END === "number");
assert(typeof std.EOF === "number");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb+");
	assert(std.fclose(fd) === 0);
}, "file open/close");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb");
	var data = toStrBuffer(phrase);
	var wsize = std.fwrite(data, 1, data.byteLength, fd);
	assert(wsize === data.byteLength);
	std.fclose(fd);
}, "file write");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "rb");
	std.fseek(fd, 0, std.SEEK_END);
	var size = std.ftell(fd);
	std.fseek(fd, 0, std.SEEK_SET);
	var data = new ArrayBuffer(size);;
	var rsize = std.fread(data, 1, data.byteLength, fd);
	assert(rsize === data.byteLength);
	var str = toStrFromBuffer(data);;
	assert(str === phrase);
	std.fclose(fd);
}, "file read");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt");
	assert(std.feof(fd) === 0);
	std.fseek(fd, 0, std.SEEK_END);
	try {
		std.fread(new ArrayBuffer(4), 1, 4, fd);
	}
	catch(err) {}
	assert(std.feof(fd) !== 0);
	std.fclose(fd);
}, "feof");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb");
	assert(std.ferror(fd) === 0);
	try {
		/* reading on write stream */
		std.fread(new ArrayBuffer(4), 1, 4, fd);
	}
	catch(err) {}
	assert(std.ferror(fd) !== 0);
	std.fclose(fd);
}, "ferror");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb");
	var data = toStrBuffer(phrase);
	var wsize = std.fwrite(data, 1, data.byteLength, fd);
	assert(wsize === data.byteLength);
	assert(std.fflush(fd) !== std.EOF);
	std.fclose(fd);
}, "fflush");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "rb");
	for (var i = 0; i < phrase.length; ++i) {
		var c = std.fgetc(fd);
		assert(c !== std.EOF);
		assert(c === phrase[i].charCodeAt(0));
	}
	std.fclose(fd);
}, "fgetc");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb");
	var data = toStrBuffer(phrase + "\n");
	std.fwrite(data, 1, data.byteLength, fd);
	var data = toStrBuffer(phrase + "\n");
	std.fwrite(data, 1, data.byteLength, fd);
	std.fclose(fd);
	var fd = std.fopen("./test_libstd_file.txt", "rb");
	var ab = new ArrayBuffer(128);
	std.fgets(ab, 128, fd);
	var str = toStrZFromBuffer(ab);
	assert(str === phrase + "\n");
	std.fgets(ab, 128, fd);
	var str = toStrZFromBuffer(ab);
	assert(str === phrase + "\n");
}, "fgets");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb");
	assert(std.fputc(97, fd) !== std.EOF);
	assert(std.fputc(98, fd) !== std.EOF);
	assert(std.fputc(99, fd) !== std.EOF);
	std.fclose(fd);
	var fd = std.fopen("./test_libstd_file.txt", "rb");
	assert(std.fgetc(fd) === 97);
	assert(std.fgetc(fd) === 98);
	assert(std.fgetc(fd) === 99);
	std.fclose(fd);
}, "fputc");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb");
	assert(std.fputs(toStrBuffer(phrase + "\n"), fd) !== std.EOF);
	assert(std.fputs(toStrBuffer(phrase + "\n"), fd) !== std.EOF);
	std.fclose(fd);
	var fd = std.fopen("./test_libstd_file.txt", "rb");
	var ab = new ArrayBuffer(128);
	assert(std.fgets(ab, 128, fd) !== std.EOF);
	assert(toStrZFromBuffer(ab) === phrase + "\n");
	assert(std.fgets(ab, 128, fd) !== std.EOF);
	assert(toStrZFromBuffer(ab) === phrase + "\n");
	std.fclose(fd);
}, "fputs");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file_copy.txt", "wb+");
	assert(std.fputs(toStrBuffer(phrase + "\n"), fd) !== std.EOF);
	assert(std.fputs(toStrBuffer(phrase + "\n"), fd) !== std.EOF);
	std.fclose(fd);
	std.remove("./test_libstd_file_copy.txt");
}, "remove");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file_copy.txt", "wb+");
	assert(std.fputs(toStrBuffer(phrase + "\n"), fd) !== std.EOF);
	assert(std.fputs(toStrBuffer(phrase + "\n"), fd) !== std.EOF);
	std.fclose(fd);
	std.rename("./test_libstd_file_copy.txt", "./test_libstd_file_copy2.txt");
	std.remove("./test_libstd_file_copy2.txt");
}, "rename");

noexcept(function() {
	var fd = std.fopen("./test_libstd_file.txt", "wb+");
	assert(std.fputs(toStrBuffer(phrase), fd) !== std.EOF);
	std.rewind(fd);
	assert(std.ftell(fd) === 0);
	std.fclose(fd);
}, "rewind");

noexcept(function() {
	var fd = std.tmpfile();
	std.fclose(fd);
}, "tmpfile");

noexcept(function() {
	var cwd = std.getcwd();
	assert(typeof cwd === "string");
}, "getcwd");

noexcept(function() {
	var user = std.getenv("USER");
	assert(typeof user === "string");
}, "getenv");

noexcept(function() {
	var wsize = std.writeFile("./test_libstd_file.txt", phrase);
	assert(wsize === phrase.length);
	var result = std.readFile("./test_libstd_file.txt");
	assert(result === phrase);
}, "readFile, writeFile");

noexcept(function() {
	assert(std.mkdir("./abcdefg") === 0);
	std.remove("./abcdefg");
}, "mkdir");

noexcept(function() {
	assert(std.mkdir("./abcdefg") === 0);
	assert(std.writeFile("./abcdefg/file1.txt", "hello"));
	assert(std.writeFile("./abcdefg/file2.txt", "hello"));
	var arr = std.readdir("./abcdefg");
	std.remove("./abcdefg/file1.txt");
	std.remove("./abcdefg/file2.txt");
	std.remove("./abcdefg");
	assert(arr.indexOf("file1.txt") >= 0);
	assert(arr.indexOf("file2.txt") >= 0);
}, "readdir");

noexcept(function() {
	var result = std.stat("./test_libstd_file.txt");
	assert(result.isFile && !result.isDirectory);
	assert(result.size >= 12);
	try {
		std.mkdir("./abcdefg");
	} catch (err) {}
	var result = std.stat("./abcdefg");
	std.remove("./abcdefg");
	assert(result.isDirectory && !result.isFile);
}, "stat");

noexcept(function() {
	assert(std.exists("./test_libstd_file.txt"));
	try {
		std.mkdir("./abcdefg");
	} catch (err) {}
	var result = std.exists("./abcdefg", true);
	std.remove("./abcdefg");
	assert(result);
	assert(std.exists("./test_libstd_file.txt", true) === false);
	assert(std.exists("./abcdefg", true) === false);
}, "exists");

noexcept(function() {
	assert(std.mkdir("./abcdefg") === 0);
	assert(std.writeFile("./abcdefg/file1.txt", "hello"));
	assert(std.writeFile("./abcdefg/file2.txt", "hello"));
	std.rmtree("./abcdefg");
	assert(std.exists("./abcdefg", true) === false);
}, "rmtree");


