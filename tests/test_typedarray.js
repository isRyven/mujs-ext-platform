function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
function noexcept(f, m) { try { f() } catch (e) { throw new Error(e || m); } }

var ab = new ArrayBuffer(8);
assert(Int8Array);
assert(Int16Array);
assert(Int32Array);
assert(Uint8Array);
assert(Uint16Array);
assert(Uint32Array);
assert(Float32Array);
assert(Float64Array);
assert(BigInt64Array);
assert(BigUint64Array);
assert(Uint8ClampedArray);
assert(Int16Array.BYTES_PER_ELEMENT == 2);
assert(Int16Array.name == "Int16Array");
var ta = new Int16Array(ab);
assert(ta.BYTES_PER_ELEMENT = 2);
assert(ta.name === undefined);
assert(ta.byteOffset === 0);
assert(ta.byteLength === 8);
assert(ta.buffer === ab);
assert(ta.length === 4);
/* non positive byte offset throw exception */
except(function() { new  Int16Array(ab, -10) });
/* non multiple of the element size byte offset should throw an error */
except(function() { new  Int16Array(ab, 3) });
/* oob byte offsets should throw an error */
except(function() { new  Int16Array(ab, 9) });
/* non aligned byte length should throw an exception */
except(function() { new  Int16Array(new ArrayBuffer(9), 2) });
/* oob array length should throw an exception */
except(function() { new  Int16Array(ab, 0, 5) });	
/* test overload that should initialize own buffer object */
var ta = new Int16Array(4);
assert(ta.buffer instanceof ArrayBuffer);
assert(ta.byteLength === 8);
assert(ta.byteOffset === 0);
assert(ta.length === 4);
var ta = new Int16Array(4);
ta[0] = 512;
ta[1] = 1024;
ta[2] = 2048;
ta[3] = 4096;
assert(ta[0] === 512 && ta[1] === 1024 && ta[2] === 2048 && ta[3] === 4096);
var ta = Int16Array.of(1, 2, 3, 4);
assert(ta[0] === 1 && ta[1] === 2 && ta[2] === 3 && ta[3] === 4);
var ta2 = Int16Array.from(ta);
assert(ta2[0] === 1 && ta2[1] === 2 && ta2[2] === 3 && ta2[3] === 4);
/* needs something that can be iterated upon */
except(function() { Int16Array.from(10); });
assert(ta2.forEach instanceof Function);
assert(ta2.find instanceof Function);
assert(ta2.findIndex instanceof Function);
assert(ta2.filter instanceof Function);
assert(ta2.map instanceof Function);
assert(ta2.every instanceof Function);
assert(ta2.some instanceof Function);
assert(ta2.subarray instanceof Function);
assert(ta2.fill instanceof Function);
assert(ta2.includes instanceof Function);
assert(ta2.set instanceof Function);
assert(ta2.indexOf instanceof Function);
assert(ta2.lastIndexOf instanceof Function);
assert(ta2.join instanceof Function);
assert(ta2.reverse instanceof Function);
assert(ta2.reduce instanceof Function);
assert(ta2.reduceRight instanceof Function);
assert(ta2.slice instanceof Function);
assert(ta2.copyWithin instanceof Function);
assert(ta2.keys instanceof Function);
assert(ta2.values instanceof Function);
assert(ta2.entries instanceof Function);
assert(ta2.sort instanceof Function);

noexcept(function() {
	ta2.forEach(function(el, i) { if (el !== ta2[i]) throw new Error(); });
}, "forEach");

noexcept(function() {
	var value = ta2.find(function(val) { return val === 2 });
	assert(value === 2);

	noexcept(function() {
		var value = ta.find(function(val) { return false });
		assert(value === undefined);	
	}, "find failed");
}, "find");

noexcept(function() {
	var index = ta2.findIndex(function(val) { return val === 2 });
	assert(index === 1);

	noexcept(function() {
		var value = ta.findIndex(function(val) { return false });
		assert(value === -1);
	}, "findIndex failed");
}, "findIndex");

noexcept(function() {
	var newTypedArray = ta.filter(function(val) { return val > 2; });
	assert(newTypedArray instanceof Int16Array);
	assert(newTypedArray.byteOffset === 0);
	assert(newTypedArray.length === 2);
	assert(newTypedArray.buffer !== ta.buffer);
	assert(newTypedArray[0] === 3);
	assert(newTypedArray[1] === 4);

	noexcept(function() {
		var newTypedArray = ta.filter(function(val) { return false });
		assert(newTypedArray instanceof Int16Array);
		assert(newTypedArray.length === 0);
	}, "filter failed");
}, "filter");

noexcept(function() {
	var newTypedArray = ta.map(function(val) { return val * 10; });
	assert(newTypedArray instanceof Int16Array);
	assert(newTypedArray.byteOffset === 0);
	assert(newTypedArray.length === 4);
	assert(newTypedArray.buffer !== ta.buffer);
	assert(newTypedArray[0] === 10);
	assert(newTypedArray[1] === 20);
	assert(newTypedArray[2] === 30);
	assert(newTypedArray[3] === 40);
}, "map");

noexcept(function() {
	var result = ta.every(function(val) { return val > 2 });
	assert(result === false);
	var result = ta.every(function(val) { return val > 0 });
	assert(result === true);
}, "every");

noexcept(function() {
	var result = ta.some(function(val) { return val > 2 });
	assert(result === true);
	var result = ta.some(function(val) { return val > 4 });
	assert(result === false);
}, "some");

noexcept(function() {
	var ab = new ArrayBuffer(16);
	var ta = new Int16Array(ab);
	ta[4] = 1;
	ta[5] = 2;
	ta[6] = 3;
	ta[7] = 4;
	var ta2 = ta.subarray(4, 8);
	assert(ta2.buffer === ta.buffer);
	assert(ta2.byteOffset === 8);
	assert(ta2.length === 4);
	assert(ta2[0] === ta[4]);
	assert(ta2[1] === ta[5]);
	assert(ta2[2] === ta[6]);
	assert(ta2[3] === ta[7]);
	ta[4] = 13;
	assert(ta2[0] === 13);
	ta2[0] = 16;
	assert(ta[4] === 16);

	noexcept(function() {
		var ta2 = ta.subarray(-4);
		assert(ta2.byteOffset === 8);
		assert(ta2.length === 4);
	});

	noexcept(function() {
		var ta2 = ta.subarray(-6, -2);
		assert(ta2.byteOffset === 4);
		assert(ta2.length === 4);
	});	
}, "subarray");

noexcept(function () {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 1; ta[1] = 2; ta[2] = 3; ta[3] = 4;
	ta.fill(13);
	assert(ta[0] === 13 && ta[1] === 13 && ta[2] === 13 && ta[3] === 13);
	
	ta[0] = 1; ta[1] = 2; ta[2] = 3; ta[3] = 4;
	ta.fill(13, 2);
	assert(ta[0] === 1 && ta[1] === 2 && ta[2] === 13 && ta[3] === 13);

	ta[0] = 1; ta[1] = 2; ta[2] = 3; ta[3] = 4;
	ta.fill(13, -2);
	assert(ta[0] === 1 && ta[1] === 2 && ta[2] === 13 && ta[3] === 13);
}, "fill");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	assert(ta.includes(10));
	assert(!ta.includes(50));
	assert(ta.includes(20, -3));
	assert(!ta.includes(20, -2));
}, "includes");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	assert(ta.indexOf(10) == 0);
	assert(ta.indexOf(50) === -1);
	assert(ta.indexOf(20, -3) === 1);
	assert(ta.indexOf(20, -2) === -1);
}, "indexOf");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 20; ta[1] = 10; ta[2] = 20; ta[3] = 40;
	assert(ta.lastIndexOf(20) === 2);
	assert(ta.lastIndexOf(50) === -1);
	assert(ta.lastIndexOf(20, -3) === 2);
}, "lastIndexOf");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 20; ta[1] = 10; ta[2] = 20; ta[3] = 40;
	var str = ta.join();
	assert(str === "20,10,20,40");
	var str = ta.join(" ");
	assert(str === "20 10 20 40");
}, "join");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	ta.reverse();
	assert(ta[0] === 40 && ta[1] === 30 && ta[2] === 20 && ta[3] === 10);
}, "reverse");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	var re = ta.reduce(function(acc, val) { return acc + val });
	assert(re === 100);
	var re = ta.reduce(function(acc, val) { return acc + val }, 100);
	assert(re === 200);
}, "reduce");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 100;
	var re = ta.reduceRight(function(acc, val) { return acc - val });
	assert(re === 40);
	var re = ta.reduce(function(acc, val) { return acc - val }, 210);
	assert(re === 50);
}, "reduceRight");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	var ta2 = ta.slice(-2);
	assert(ta2.buffer !== ta.buffer);
	assert(ta2[0] === 30 && ta2[1] === 40);
	assert(ta2.length === 2);
	assert(ta2.byteOffset === 0);
}, "slice");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	ta.copyWithin(2, 0, 2);
	assert(ta[2] === 10 && ta[3] === 20);
}, "copyWithin");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	var it = ta.keys();
	assert(typeof it === "object");
	assert(it.next instanceof Function);
	var re = it.next();
	assert(typeof re === "object");
	assert(re.value !== undefined && re.done !== undefined);
	assert(re.value === 0 && re.done === false);
	var re = it.next();
	assert(re.value === 1 && re.done === false);
	var re = it.next();
	assert(re.value === 2 && re.done === false);
	var re = it.next();
	assert(re.value === 3 && re.done === false);
	var re = it.next();
	assert(re.value === undefined && re.done === true);
}, "keys");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	var it = ta.values();
	assert(typeof it === "object");
	assert(it.next instanceof Function);
	var re = it.next();
	assert(typeof re === "object");
	assert(re.value !== undefined && re.done !== undefined);
	assert(re.value === 10 && re.done === false);
	var re = it.next();
	assert(re.value === 20 && re.done === false);
	var re = it.next();
	assert(re.value === 30 && re.done === false);
	var re = it.next();
	assert(re.value === 40 && re.done === false);
	var re = it.next();
	assert(re.value === undefined && re.done === true);
}, "values");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	var it = ta.entries();
	assert(typeof it === "object");
	assert(it.next instanceof Function);
	var re = it.next();
	assert(typeof re === "object");
	assert(re.value !== undefined && re.done !== undefined);
	assert(re.value instanceof Array);
	assert(re.value == "0,10" && re.done === false);
	var re = it.next();
	assert(re.value == "1,20" && re.done === false);
	var re = it.next();
	assert(re.value == "2,30" && re.done === false);
	var re = it.next();
	assert(re.value == "3,40" && re.done === false);
	var re = it.next();
	assert(re.value === undefined && re.done === true);
}, "entries");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 40; ta[1] = 30; ta[2] = 20; ta[3] = 10;
	ta.sort();
	assert(ta[0] === 10 && ta[1] === 20 && ta[2] === 30 && ta[3] === 40);
	ta.sort(function(val1, val2) {
		return val2 - val1;
	});
	assert(ta[0] === 40 && ta[1] === 30 && ta[2] === 20 && ta[3] === 10);
}, "sort");

noexcept(function() {
	var ta = new Int16Array(new ArrayBuffer(8));
	ta[0] = 10; ta[1] = 20; ta[2] = 30; ta[3] = 40;
	ta.set([50, 60], 2); /* array */
	assert(ta[2] === 50 && ta[3] === 60);
	var ta2 = new Int16Array(2); /* same kinds */
	ta2[0] = 30; ta2[1] = 40;
	ta.set(ta2);
	assert(ta[0] === 30 && ta[1] === 40 && ta[2] === 50 && ta[3] === 60);
	var ta3 = new Int32Array(2); /* different kinds */
	ta3[0] = 1024; ta3[1] = 2048;
	ta.set(ta3, 2);
	assert(ta[2] === 1024 && ta[3] === 2048);
}, "set");
