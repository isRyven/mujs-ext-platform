function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
function noexcept(f, m) { try { f() } catch (e) { throw new Error(e || m); } }

assert(typeof VM === "object");
assert(typeof VM.compile === "function", "should contain compile method");
except(function() {
	VM.compile();
}, "should throw error if no source parm is set");

except(function() {
	VM.compile(123);
}, "should throw error if source is not of valid type");

noexcept(function() {
	VM.compile("var abc = 5");
	VM.compile(allocString("var abc = 5"));
});

assert(typeof VM.compile("var abc = 5") === "object", "should return script object");
assert(typeof VM.compile(allocString("var abc = 5")) === "object", "should return script object");

noexcept(function() {
	var mod = VM.compile("var abc = 13");
	mod();
})

except(function() {
	var mod = VM.compile("throw new Error('error')");
	mod();
});

noexcept(function() {
	var mod = VM.compile("if (abc !== 13) throw new Error('error')", 'test', { abc: 13 });
	mod();
});

