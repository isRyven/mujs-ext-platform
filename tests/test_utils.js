function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
function noexcept(f, m) { try { f() } catch (e) { throw new Error(e || m); } }

assert(typeof Utils === "object");
assert(typeof Utils.toString === "function", "should contain compile method");

noexcept(function() {
	assert(Utils.toString() === "");
	assert(Utils.toString("string") === "string");
	/* accepts ArrayBuffer */
	assert(Utils.toString(allocString("string")) === "string");
	assert(Utils.toString("str", "ascii") === "str");
	assert(Utils.toString("str", "utf8") === "str");
	except(function() {
		Utils.toString("str", "");
	});
	assert(Utils.toString("täst", "utf8").length === 4);
	except(function() {
		assert(Utils.toString("täst", "ascii").length === 4);
	})
});