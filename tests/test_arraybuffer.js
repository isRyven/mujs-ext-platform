function assert(t, m) { if (!t) throw new Error(m); }
function except(f, m) { try { f() } catch (e) { return; }; throw new Error(m); }
/* test init */
var buffer = new ArrayBuffer(0);
/* test bytel ength */
var buffer = new ArrayBuffer(4);
assert(buffer.byteLength == 4, "array buffer has incorrect size");
var buffer = new ArrayBuffer(16);
assert(buffer.byteLength == 16, "array buffer has incorrect size");
var buffer = new ArrayBuffer(32);
assert(buffer.byteLength == 32, "array buffer has incorrect size");
var buffer = new ArrayBuffer(64);
assert(buffer.byteLength == 64, "array buffer has incorrect size");
var buffer = new ArrayBuffer(128);
assert(buffer.byteLength == 128, "array buffer has incorrect size");
/* test slice */
var buffer = new ArrayBuffer(8);
var bufferB = buffer.slice(4);
assert(bufferB, "slice did not return valid object");
assert(bufferB != buffer, "slice did not return copy of the object");
assert(bufferB.byteLength == 4, "array buffer has incorrect size");
var bufferB = buffer.slice();
assert(bufferB.byteLength == 8, "array buffer has incorrect size");
/* test ranges */
var bufferB = buffer.slice(2, 6);
assert(bufferB.byteLength == 4, "array buffer has incorrect size");
/* test negative values */
var bufferB = buffer.slice(-4);
assert(bufferB.byteLength == 4, "array buffer has incorrect size");
var bufferB = buffer.slice(-6, -2);
assert(bufferB.byteLength == 4, "array buffer has incorrect size");
/* test byteLength is non writable */
var bufferB = buffer.slice(0);
bufferB.byteLength = 10;
assert(bufferB.byteLength == 8, "array buffer has incorrect size");
/* test invalid range */
var bufferB = buffer.slice(-1000, -1000);
assert(bufferB.byteLength == 0, "array buffer has incorrect size");
/* test incorrect range passed in constructor  */
except(function() { new ArrayBuffer(-10); }, "array buffer constructor should throw error");
/* should be okay to pass 0 */
var bufferB = new ArrayBuffer(0);
/* should instantiate ArrayBuffer only using new keyword  */
except(function() { ArrayBuffer(10); });
