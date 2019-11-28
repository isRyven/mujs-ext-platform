function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
function noexcept(f, m) { try { f() } catch (e) { throw new Error(e || m); } }

assert(typeof ReadStream === "function");

noexcept(function() {
	var stream = new ReadStream();
	assert(typeof stream.push === "function");
	assert(typeof stream.read === "function");
	assert(typeof stream.addEventListener === "function");
	assert(typeof stream.removeEventListener === "function");
});

noexcept(function() {
	var rstream = new ReadStream();
	var isDataListnerCalled = false;
	var isClosedListenerCalled = false;
	var isDataReadWasCalled = false;
	rstream.addEventListener("data", function() {
		isDataListnerCalled = true;
	});
	rstream.addEventListener("close", function() {
		isClosedListenerCalled = true;
	});
	rstream.push("hello world, ");
	assert(!isDataListnerCalled && !isClosedListenerCalled);
	rstream.push("world hello");
	assert(!isDataListnerCalled && !isClosedListenerCalled);
	rstream.end();
	assert(!isDataListnerCalled);  /* should be async */  
	eventloop.poll(0);
	assert(isDataListnerCalled && !isClosedListenerCalled);
	rstream.addEventListener("data", function() {
		isDataListnerCalled = true;
		var data = rstream.read();
		assert(data instanceof ArrayBuffer);
		assert(data.byteLength === 24);
		var str = Utils.toString(data);
		assert(str.length === 24);
	});
	rstream.end(); /* consume data */
	rstream.close(); /* should be async */
	assert(!isClosedListenerCalled);
	eventloop.poll(0);
	assert(isDataListnerCalled);
	assert(!isClosedListenerCalled);
	eventloop.poll(0);
	assert(isClosedListenerCalled);
});

noexcept(function() {
	var rstream = new ReadStream();
	rstream.addEventListener("data", function() {
		var chunk1 = rstream.read(6);
		assert(chunk1.byteLength === 6);
		assert(Utils.toString(chunk1) === "hello,");
		var chunk2 = rstream.read(6);
		assert(chunk2.byteLength === 6);
		assert(Utils.toString(chunk2) === " there");
		var chunk3 = rstream.read(5);
		assert(chunk3.byteLength === 5);
		assert(Utils.toString(chunk3) === "world");
		var chunk4 = rstream.read(7);
		assert(chunk4.byteLength === 7);
		assert(Utils.toString(chunk4) === ", hello");
	});
	rstream.push("hello, there");
	rstream.push("world, hello");
	rstream.end();
	eventloop.poll(0);
});

noexcept(function() {
	var rstream = new ReadStream();
	rstream.addEventListener("data", function() {
		var chunk1 = rstream.read(12);
		assert(chunk1.byteLength === 12);
		assert(Utils.toString(chunk1) === "hello, there");
		var chunk2 = rstream.read(12);
		assert(chunk2.byteLength === 12);
		assert(Utils.toString(chunk2) === "world, hello");
	});
	rstream.push("hello, there");
	rstream.push("world, hello");
	rstream.end();
	eventloop.poll(0);
});

noexcept(function() {
	var rstream = new ReadStream();
	rstream.addEventListener('data', function() {
		var chunk1 = rstream.read(12);
		assert(chunk1.byteLength === 12);
		assert(Utils.toString(chunk1) === "world, hello");
		var chunk2 = rstream.read(12);
		assert(chunk2.byteLength === 12);
		assert(Utils.toString(chunk2) === "hello, there");
	});
	rstream.shift('hello, there');
	rstream.shift('world, hello');
	rstream.end();
	eventloop.poll(0);
});

noexcept(function() {
	var rstream = new ReadStream();
	var reader1 = function() {
		var chunk = rstream.read(6);
		assert(chunk.byteLength === 6);
		assert(Utils.toString(chunk) === "hello,");
	}
	rstream.addEventListener("data", reader1);
	rstream.push("hello, world");
	rstream.end();
	eventloop.poll(0);
	rstream.removeEventListener("data", reader1);
	var reader2 = function() {
		var chunk = rstream.read();
		assert(chunk.byteLength === 12);
		assert(Utils.toString(chunk) === "wasup, world");
	}
	rstream.addEventListener("data", reader2);
	rstream.shift("wasup,");
	rstream.end();
	eventloop.poll(0);
});
