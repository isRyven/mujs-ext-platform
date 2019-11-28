function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
function noexcept(f, m) { try { f() } catch (e) { throw new Error(e || m); } }

var ev = new EventLoop();
assert(ev.addHandler);
assert(ev.removeHandler);
assert(ev.poll);

except(function() {
	var ev = new EventLoop();
	ev.addTimerHandler(null);
});

except(function() {
	var ev = new EventLoop();
	ev.addTimerHandler(null);
});

noexcept(function() {
	var ev = new EventLoop();
	assert(typeof ev.addHandler({ type: "timer", delay: 0, handler: function() {} }) == "number");
});

noexcept(function() {
	var ev = new EventLoop();
	assert(typeof ev.addHandler({ type: "io", handler: function() {} }) == "number");
});

noexcept(function() {
	var ev = new EventLoop();
	// assert(typeof ev.poll() === "number");
});

noexcept(function() {
	var ev = new EventLoop();
	var id = ev.addHandler({
		type: "timer",
		delay: 100,
		handler: function() {},
		persistent: false
	});
	assert(id == 1);
	var id = ev.addHandler({
		type: "timer",
		delay: 100,
		handler: function() {},
		persistent: false
	});
	assert(id == 2);
});

noexcept(function() {
	var ev = new EventLoop();
	var id = ev.addHandler({
		type: "timer",
		delay: 100,
		handler: function() {},
		persistent: false
	});
	assert(ev.removeHandler(id));
});


noexcept(function() {
	var ev = new EventLoop();
	var wasCalled = false;
	var id = ev.addHandler({
		type: 'timer',
		delay: 10,
		handler: function() {
			wasCalled = true;
		},
		persistent: false
	});
	assert(id == 1);
	var then = Date.now();
	assert(ev.poll(0) === 1);
	var now = Date.now();
	assert(wasCalled === false);
	assert(now - then == 0);
}, "should not call handler if timeout is set to 0 (nonblocking)");

noexcept(function() {
	var ev = new EventLoop();
	var wasCalled = false;
	var id = ev.addHandler({
		type: 'timer',
		delay: 10,
		handler: function() {
			wasCalled = true;
		},
		persistent: false
	});
	assert(id == 1);
	var then = Date.now();
	assert(ev.poll(10) === 1);
	var now = Date.now();
	assert(wasCalled === true);
	assert(now - then >= 10);
	assert(ev.poll(0) == 0);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var secondWasCalled = false;
	var id = ev.addHandler({
		type: 'timer',
		delay: 10,
		handler: function() {
			firstWasCalled = true;
		},
		persistent: false
	});
	assert(id == 1);
	var id = ev.addHandler({
		type: 'timer',
		delay: 5,
		handler: function() {
			secondWasCalled = true;
		},
		persistent: false
	});
	assert(id == 2);
	
	var then = Date.now();
	assert(ev.poll(10) === 1);
	var now = Date.now();
	assert(secondWasCalled === true);
	assert((now - then) >= 4 && (now - then) <= 7);

	var then = Date.now();
	assert(ev.poll(10) === 1);
	var now = Date.now();
	assert(firstWasCalled === true);
	assert((now - then) >= 4 && (now - then) <= 7);

	var then = Date.now();
	assert(ev.poll(10) == 0);
	var now = Date.now();
	assert(now - then <= 1);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var id = ev.addHandler({
		type: 'timer',
		delay: 15,
		handler: function() {
			firstWasCalled = true;
		},
		persistent: false
	});
	assert(id == 1);

	var then = Date.now();
	assert(ev.poll(10) === 1);
	var now = Date.now();
	assert(firstWasCalled === false);
	assert((now - then) >= 10 && (now - then) <= 11);

	var then = Date.now();
	assert(ev.poll(10) === 1);
	var now = Date.now();
	assert(firstWasCalled === true);
	assert((now - then) >= 4 && (now - then) <= 7);	

	var then = Date.now();
	assert(ev.poll(10) == 0);
	var now = Date.now();
	assert(now - then <= 1);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var secondWasCalled = false;
	var thirdWasCalled = false;
	ev.addHandler({
		type: 'timer',
		delay: 15,
		handler: function() {
			firstWasCalled = true;
		},
		persistent: false
	});
	ev.addHandler({
		type: 'timer',
		delay: 25,
		handler: function() {
			secondWasCalled = true;
		},
		persistent: false
	});
	ev.addHandler({
		type: 'timer',
		delay: 5,
		handler: function() {
			thirdWasCalled = true;
		},
		persistent: false
	});
	var then = Date.now();
	assert(ev.poll(-1) == 1);
	var now = Date.now();
	assert((now - then) >= 5 && (now - then) <= 6);
	assert(!firstWasCalled && !secondWasCalled && thirdWasCalled);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	ev.addHandler({
		type: 'timer',
		delay: 0,
		arguments: [13, 16],
		handler: function(arg1, arg2) {
			firstWasCalled = true;
			print(arg1, arg2);
			assert(arg1 === 13);
			assert(arg2 === 16);
		},
		persistent: false
	});
	ev.poll(0);
	assert(firstWasCalled);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var expectedTimeout = 0;
	var counter = 0;
	var id = ev.addHandler({
		type: 'io',
		handler: function(timeout) {
			expectedTimeout = timeout;
			if (counter++ == 2) {
				firstWasCalled = true;
				return 1;
			}
			return 0;
		},
		persistent: false
	});
	assert(id == 1);
	
	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 10 && (now - then) <= 11);
	assert(!firstWasCalled);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 10 && (now - then) <= 11);
	assert(!firstWasCalled);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert(firstWasCalled);
	print(now - then);
	assert((now - then) >= 0 && (now - then) <= 1);	

	assert(ev.poll(10) == 0);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var expectedTimeout = 0;
	var counter = 0;
	var id = ev.addHandler({
		type: 'io',
		handler: function(timeout) {
			expectedTimeout = timeout;
			if (counter++ == 2) {
				firstWasCalled = true;
				return 1;
			}
			return 0;
		},
		persistent: true
	});
	assert(id == 1);
	
	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 10 && (now - then) <= 11);
	assert(!firstWasCalled);
	print(expectedTimeout);	
	assert(expectedTimeout == 10);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 10 && (now - then) <= 11);
	assert(!firstWasCalled);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert(firstWasCalled);
	print(now - then);
	assert((now - then) >= 0 && (now - then) <= 1);	

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	print(now - then);
	assert((now - then) >= 10 && (now - then) <= 11);	
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var secondWasCalled = false;
	var thirdWasCalled = false;
	var timeoutValue = 0;
	var counter = 0;
	var id = ev.addHandler({
		type: 'timer',
		delay: 10,
		handler: function() {
			firstWasCalled = true;
		},
		persistent: false
	});
	assert(id == 1);

	var id = ev.addHandler({
		type: 'io',
		handler: function(timeout) {
			secondWasCalled = true;
			timeoutValue = timeout;
			return 1;
		},
		persistent: false
	})
	assert(id == 2);

	var id = ev.addHandler({
		type: 'io',
		handler: function(timeout) {
			thirdWasCalled = true;
			timeoutValue = timeout;
			return 1;
		},
		persistent: false
	})
	assert(id == 3);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 0 && (now - then) <= 1);
	assert(!firstWasCalled && (secondWasCalled || thirdWasCalled));

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 0 && (now - then) <= 1);
	assert(!firstWasCalled && (secondWasCalled || thirdWasCalled));

	secondWasCalled = false;
	thirdWasCalled = false;
	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 9 && (now - then) <= 11);
	assert(firstWasCalled && !secondWasCalled && !thirdWasCalled);

	assert(ev.poll(10) == 0);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var secondWasCalled = false;
	var counter = 0;
	var id = ev.addHandler({
		type: 'timer',
		delay: 10,
		handler: function() {
			firstWasCalled = true;
		},
		persistent: false
	});
	assert(id == 1);

	var id = ev.addHandler({
		type: 'io',
		handler: function(timeout) {
			sleep(5);
			secondWasCalled = true;
			return 1;
		},
		persistent: false
	})
	assert(id == 2);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 4 && (now - then) <= 7);
	assert(!firstWasCalled && secondWasCalled);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 4 && (now - then) <= 7);
	assert(firstWasCalled && (secondWasCalled));

	assert(ev.poll(10) == 0);
});

noexcept(function() {
	var ev = new EventLoop();
	var firstWasCalled = false;
	var secondWasCalled = false;
	var counter = 0;
	var id = ev.addHandler({
		type: 'timer',
		delay: 10,
		handler: function() {
			firstWasCalled = true;
		},
		persistent: false
	});
	assert(id == 1);

	var id = ev.addHandler({
		type: 'io',
		handler: function(timeout) {
			sleep(5);
			secondWasCalled = true;
			return 0;
		},
		persistent: false
	})
	assert(id == 2);

	var then = Date.now();
	assert(ev.poll(10) == 1);
	var now = Date.now();
	assert((now - then) >= 9 && (now - then) <= 12);
	assert(firstWasCalled && secondWasCalled);
});

