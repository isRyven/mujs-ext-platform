function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
function noexcept(f, m) { try { f() } catch (e) { throw new Error(e || m); } }

assert(typeof EventEmitter === "function");

noexcept(function() {
	var ev = new EventEmitter();
	assert(typeof ev.addEventListener === "function");
	assert(typeof ev.removeEventListener === "function");
	assert(typeof ev.emit === "function");

	ev.addEventListener("hello", function() {});
	ev.addEventListener("hello", function() {});
	ev.addEventListener("hello", function() {});
});

noexcept(function() {
	var ev = new EventEmitter();
	var wasCalled = 0;
	var listener = function() { wasCalled += 1 };
	ev.addEventListener("hello", listener);
	ev.emit("hello");
	assert(wasCalled === 1);
	ev.emit("hello");
	assert(wasCalled === 2);
	ev.removeEventListener("hello", listener);
	ev.emit("hello");
	assert(wasCalled !== 3);
});

noexcept(function() {
	var ev = new EventEmitter();
	var wasCalled = 0;
	var listener = function() { wasCalled += 1 };
	var cancel = ev.addEventListener("hello", listener);
	ev.emit("hello");
	ev.emit("hello");
	assert(wasCalled === 2);
	cancel();
	ev.emit("hello");
	assert(wasCalled === 2);
});

noexcept(function() {
	var ev = new EventEmitter();
	var wasCalled = 0;
	ev.addEventListener('hello', function() { wasCalled += 1 });
	ev.emit('hello');
	ev.removeEventListener('hello', function() {});
	ev.emit('hello');
	ev.emit('hello');
	assert(wasCalled === 3);
});
