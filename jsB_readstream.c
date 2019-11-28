#include "jsB_readstream.h"
#include "jsB_arraybuffer.h"
#include "jsB_eventemitter.h"
#include "jsB_utils.h"
#include "jsB_timers.h"
#include "jsB_typedarray.h"

#include <stdio.h>

#define U_NAME "ReadStream"

static void jsB_Node(js_State *J)
{
	/* node instance */
	js_newobject(J);
	js_copy(J, 1);
	js_setproperty(J, -2, "value"); /* value we store in the node */
	js_pushnull(J);
	js_setproperty(J, -2, "next"); /* reference to the next node */
}

static void jsB_call_ArrayBufferList(js_State *J)
{
	js_typeerror(J, "expected new");
}

static void jsB_new_ArrayBufferList(js_State *J)
{
	js_currentfunction(J);
	js_getproperty(J, -1, "prototype");
	js_newobjectx(J);
	/* new Node(null) */
	js_pushundefined(J);
	js_pushnull(J);
	js_callscoped2(J, jsB_Node, 1);
	/* this.tail = node */
	js_copy(J, -1);
	js_setproperty(J, -3, "head");
	/* this.tail = this.head */
	js_setproperty(J, -2, "tail");
	/* this.length */
	js_pushnumber(J, 0);
	js_setproperty(J, -2, "length");
}

static void jsB_ArrayBufferList_prototype_push(js_State *J)
{
	if (!jsB_ArrayBuffer_instance(J, 1)) {
		js_newtypeerror(J, "expected ArrayBuffer instance");
	}
	/* new Node */
	js_pushundefined(J);
	js_copy(J, 1);
	js_callscoped2(J, jsB_Node, 1);
	/* this.tail.next = node */
	js_getproperty(J, 0, "tail");
	js_copy(J, -2);
	js_setproperty(J, -2, "next");
	js_pop(J, 1);
	/* this.tail = node */
	js_setproperty(J, 0, "tail");
	/* this.length++ */
	js_getproperty(J, 0, "length");
	int length = js_tointeger(J, -1);
	js_pop(J, 1);
	js_pushnumber(J, (double)(length + 1));
	js_setproperty(J, 0, "length");
	js_pushundefined(J);
}

static void jsB_ArrayBufferList_prototype_remove(js_State *J)
{
	/* implement as needed */
	/*
		if (!this.head.next) {
			return;
		}
		var node = this.head;
		var prevNode;
		var counter = 0;
		while ((node = node.next)) {
			if (index === counter) {
				if (prevNode) {
					prevNode.next = node.next;
					if (!node.next) {
						this.tail = prevNode;
					} 
				} else {
					this.head.next = node.next;
					if (!node.next) {
						this.tail = this.head;
					}
				}
				this.length--;
				break;
			}
			counter++;
			prevNode = node;
		}
		return node.value;
	*/
}

static void jsB_ArrayBufferList_prototype_shift(js_State *J)
{
	if (!jsB_ArrayBuffer_instance(J, 1)) {
		js_newtypeerror(J, "expected ArrayBuffer instance");
	}
	/* node = new Node */
	js_pushundefined(J);
	js_copy(J, 1);
	js_callscoped2(J, jsB_Node, 1);
	/* node.next = this.head.next */
	js_getproperty(J, 0, "head");
	js_getproperty(J, -1, "next");
	js_setproperty(J, -3, "next");
	/* this.head.next = node */
	js_copy(J, -2);
	js_setproperty(J, -2, "next");
	/* this.length++ */
	js_getproperty(J, 0, "length");
	int length = js_tointeger(J, -1);
	js_pop(J, 1);
	js_pushnumber(J, (double)(length + 1));
	js_setproperty(J, 0, "length");
	js_pushundefined(J);
}

static void jsB_ArrayBufferList_prototype_unshift(js_State *J)
{
	/* node = this.head.next */
	js_getproperty(J, 0, "head");
	js_getproperty(J, -1, "next"); /* node */
	if (js_isnull(J, -1)) {
		return;
	}
	int node = js_gettop(J) - 1;
	/* this.head.next = node.next  */ 
	js_getproperty(J, -1, "next");
	js_setproperty(J, -3, "next");
	/* this.tail === node */
	js_getproperty(J, 0, "tail");
	js_copy(J, -2);
	if (js_strictequal(J)) {
		/* this.tail = this.next */
		js_copy(J, -4);
		js_setproperty(J, 0, "tail");
	}
	js_pop(J, 2);
	/* this.length-- */
	js_getproperty(J, 0, "length");
	int length = js_tointeger(J, -1);
	js_pop(J, 1);
	js_pushnumber(J, (double)(length - 1));
	js_setproperty(J, 0, "length");
	/* return node */
	js_getproperty(J, node, "value");
}

static void jsB_ArrayBufferList_prototype_get(js_State *J)
{
	int index = js_tointeger(J, 1);
	int counter = 0;
	js_getproperty(J, 0, "head");
	while (1) {
		js_getproperty(J, -1, "next");
		js_rot2pop1(J);
		if (js_isnull(J, -1)) {
			break;
		}
		if (index == counter++) {
			js_getproperty(J, -1, "value");
			return;
		}
	}
	js_pushundefined(J);
}

static void jsB_ArrayBufferList(js_State *J)
{
	js_getregistry(J, "jsB_ArrayBufferList");
	if (js_isdefined(J, -1)) {
		return;
	}
	js_pop(J, 1);

	js_newobject(J); /* prototype */
	js_newcfunction(J, jsB_ArrayBufferList_prototype_push, "push", 1);
	js_setproperty(J, -2, "push");
	js_newcfunction(J, jsB_ArrayBufferList_prototype_get, "get", 1);
	js_setproperty(J, -2, "get");
	js_newcfunction(J, jsB_ArrayBufferList_prototype_remove, "remove", 1);
	js_setproperty(J, -2, "remove");
	js_newcfunction(J, jsB_ArrayBufferList_prototype_shift, "shift", 1);
	js_setproperty(J, -2, "shift");
	js_newcfunction(J, jsB_ArrayBufferList_prototype_unshift, "unshift", 0);
	js_setproperty(J, -2, "unshift");

	js_newcconstructor(J, jsB_call_ArrayBufferList, jsB_new_ArrayBufferList, "ArrayBufferList", 0);
	js_copy(J, -1);
	js_setregistry(J, "jsB_ArrayBufferList");
}

static void jsB_new_ReadStream(js_State *J)
{
	js_currentfunction(J);
	js_getproperty(J, -1, "prototype");
	/* create ReadStream instance */
	js_newuserdata(J, U_NAME, NULL, NULL);
	/* create new EventEmitter instance */
	jsB_EventEmitter(J);
	js_construct(J, 0);
	/* store it in hidden fields */
	js_copy(J, -1);
	js_setlocalregistry(J, -3, "events");
	/* prebind emit method, to avoid doing this later for eventloop */ 
	js_getproperty(J, -1, "emit");
	js_getproperty(J, -1, "bind");
	js_rot2(J);
	js_copy(J, -3);
	js_call(J, 1);
	/* store the binded event emitter method */
	js_setlocalregistry(J, -3, "emitEvent");
	/* pop the EventEmitter instance */
	js_pop(J, 1);

	jsB_ArrayBufferList(J);
	js_construct(J, 0);
	js_setlocalregistry(J, -2, "buffers");
	
	js_pushnumber(J, 0);
	js_setlocalregistry(J, -2, "rpos");
	
	js_pushnumber(J, 0);
	js_setproperty(J, -2, "length");
	
	js_pushboolean(J, 0);
	js_setproperty(J, -2, "closed");
}

static void jsB_call_ReadStream(js_State *J)
{
	js_typeerror(J, "new expected");
}

static void jsB_ReadStream_prototype_read(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	js_getproperty(J, 0, "length");
	int leftToRead = js_tointeger(J, -1);
	if (leftToRead == 0) {
		js_pushundefined(J);
		return;
	}
	int size = js_checkinteger(J, 1, leftToRead);
	if (size < leftToRead) {
		leftToRead = size;
	}
	/* buffer = new ArrayBuffer(leftToRead) */
	jsB_ArrayBuffer(J);
	js_pushnumber(J, (double)leftToRead);
	js_construct(J, 1);
	int buffer = js_gettop(J) - 1;
	/* bufferView = new Uint8Array(buffer) */
	jsB_TypedArray(J, TYPEDARRAY_UINT8);
	js_copy(J, -2);
	js_construct(J, 1);
	int bufferView = js_gettop(J) - 1;
	/* numChunks */
	js_getlocalregistry(J, 0, "buffers");
	js_getproperty(J, -1, "length");
	int numChunks = js_tointeger(J, -1);
	js_pop(J, 2);
	int offset = 0;
	for (int i = 0; i < numChunks; ++i) {
		/* get first chunk */
		js_getlocalregistry(J, 0, "buffers");
		js_getproperty(J, -1, "get");
		js_rot2(J);
		js_pushnumber(J, 0);
		js_call(J, 1);
		int chunk = js_gettop(J) - 1;
		/* get byteLength */
		js_getproperty(J, -1, "byteLength");
		int byteLength = js_tointeger(J, -1);
		js_pop(J, 1);
		/* find stuff */
		int lastElement = (leftToRead - offset) <= byteLength;
		js_getlocalregistry(J, 0, "rpos");
		int rpos = js_tointeger(J, -1);
		js_pop(J, 1);
		int chunkOffset = (i == 0) ? rpos : 0;
		int chunkLength = lastElement ? (leftToRead - offset) : byteLength;
		/* set chunk into bufferView */
		js_getproperty(J, bufferView, "set");
		js_copy(J, bufferView);
			/* chunkView */
			jsB_TypedArray(J, TYPEDARRAY_UINT8);
			js_copy(J, chunk);
			js_pushnumber(J, (double)chunkOffset);
			js_pushnumber(J, (double)chunkLength);
			js_construct(J, 3);
		/* offset */
		js_pushnumber(J, offset);
		js_call(J, 2);
		js_pop(J, 1);
		/* reset rpos */
		if ((chunkOffset + chunkLength) < byteLength) {
			/* this.rpos = leftToRead - offset */
			rpos = leftToRead - offset;
			js_pushnumber(J, rpos);
			js_setlocalregistry(J, 0, "rpos");
		} else {
			js_getlocalregistry(J, 0, "buffers");
			js_getproperty(J, -1, "unshift");
			js_rot2(J);
			js_call(J, 0);
			js_pop(J, 1);
			rpos = 0;
			/* this.rpos = 0 */
			js_pushnumber(J, rpos);
			js_setlocalregistry(J, 0, "rpos");
		}
		/* pop chunk */
		js_pop(J, 1); 
		offset += byteLength;
		if (lastElement) {
			break;
		}
	}
	/* update length */
	int length = js_getlength(J, 0);
	js_pop(J, 1);
	length -= leftToRead;
	js_setlength(J, 0, length);
	/*  */
	js_getproperty(J, 0, "closed");
	int isClosed = js_toboolean(J, -1);
	js_pop(J, 1);
	if (isClosed && length == 0) {
		jsB_Timers(J);
		js_getproperty(J, -1, "setImmediate");
		js_rot2(J);
		js_getlocalregistry(J, 0, "emitEvent");
		js_pushstring(J, "close");
		js_copy(J, 0);
		js_call(J, 3);
		js_pop(J, 1);
	}
	/* return buffer */
	js_copy(J, buffer);
}

static void jsB_ReadStream_prototype_push(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	/* toArrayBuffer(arg) */
	jsB_Utils(J);
	js_getproperty(J, -1, "toArrayBuffer");
	js_rot2(J);
	js_copy(J, 1);
	js_call(J, 1);
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
	/* increase length */
	js_getproperty(J, 0, "length");
	int length = js_tointeger(J, -1);
	js_pop(J, 1);
	js_pushnumber(J, (double)(length + backStore->length));
	js_setproperty(J, 0, "length");
	/* push chunk into buffer list */
	js_getlocalregistry(J, 0, "buffers");
	js_rot2(J);
	js_callscoped2(J, jsB_ArrayBufferList_prototype_push, 1);
}

static void jsB_ReadStream_prototype_shift(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	/* toArrayBuffer(arg) */
	jsB_Utils(J);
	js_getproperty(J, -1, "toArrayBuffer");
	js_rot2(J);
	js_copy(J, 1);
	js_call(J, 1);
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
	/* increase length */
	js_getproperty(J, 0, "length");
	int length = js_tointeger(J, -1);
	js_pop(J, 1);
	js_pushnumber(J, (double)(length + backStore->length));
	js_setproperty(J, 0, "length");
	/* push chunk into buffer list beginning */
	js_getlocalregistry(J, 0, "buffers");
	/* first check if we need to truncate existing beginning buffer */
	{
		js_getlocalregistry(J, 0, "rpos");
		int rpos = js_tointeger(J, -1);
		js_pop(J, 1);
		if (rpos > 0) {
			js_copy(J, -1);
			/* returns array buffer */
			js_callscoped2(J, jsB_ArrayBufferList_prototype_unshift, 0);
			/* makes a new copy */
			js_getproperty(J, -1, "slice");
			js_rot2(J);
			js_pushnumber(J, rpos);
			js_call(J, 1);
			/* shift it back into bufferlist */
			js_copy(J, -2);
			js_rot2(J);
			js_callscoped2(J, jsB_ArrayBufferList_prototype_shift, 1);
			js_pop(J, 1);
			/* reset rpos */
			js_pushnumber(J, 0);
			js_setlocalregistry(J, 0, "rpos");
		}
	}
	js_rot2(J);
	js_callscoped2(J, jsB_ArrayBufferList_prototype_shift, 1);
}

static void jsB_ReadStream_prototype_end(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	js_getproperty(J, 0, "closed");
	if (js_toboolean(J, -1)) {
		js_newerror(J, "cannot end closed stream");
	}
	jsB_Timers(J);
	js_getproperty(J, -1, "setImmediate");
	js_rot2(J);
	js_getlocalregistry(J, 0, "emitEvent");
	js_pushstring(J, "data");
	js_copy(J, 0);
	js_call(J, 3);
	js_pushundefined(J);
}

static void jsB_ReadStream_prototype_close(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	js_getproperty(J, 0, "closed");
	if (js_toboolean(J, -1)) {
		js_newerror(J, "stream is already closed");
	}
	/* this.closed = true */
	js_pushboolean(J, 1);
	js_setproperty(J, 0, "closed");
	/* if no data left, notify about close */
	js_getproperty(J, 0, "length");
	if (js_tointeger(J, -1) == 0) {
		jsB_Timers(J);
		js_getproperty(J, -1, "setImmediate");
		js_rot2(J);
		js_getlocalregistry(J, 0, "emitEvent");
		js_pushstring(J, "close");
		js_copy(J, 0);
		js_call(J, 3);
	}
	js_pushundefined(J);
}	

static void jsB_ReadStream_prototype_addEventListener(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "expected event name to be a string");
	}
	if (!js_iscallable(J, 2)) {
		js_typeerror(J, "expected event listener to be a function");
	}
	js_getlocalregistry(J, 0, "events");
	js_getproperty(J, -1, "addEventListener");
	js_rot2(J);
	js_copy(J, 1);
	js_copy(J, 2);
	js_call(J, 2);
}

static void jsB_ReadStream_prototype_removeEventListener(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "expected event name to be a string");
	}
	if (!js_iscallable(J, 2)) {
		js_typeerror(J, "expected event listener to be a function");
	}
	js_getlocalregistry(J, 0, "events");
	js_getproperty(J, -1, "removeEventListener");
	js_rot2(J);
	js_copy(J, 1);
	js_copy(J, 2);
	js_call(J, 2);
}

void jsB_ReadStream(js_State *J)
{
	js_getregistry(J, "jsB_ReadStream");
	if (js_isdefined(J, -1)){
		return;
	}
	js_pop(J, 1);

	js_newobject(J); /* prototype */
	js_newcfunction(J, jsB_ReadStream_prototype_read, "read", 1);
	js_setproperty(J, -2, "read");
	js_newcfunction(J, jsB_ReadStream_prototype_push, "push", 1);
	js_setproperty(J, -2, "push");
	js_newcfunction(J, jsB_ReadStream_prototype_shift, "shift", 1);
	js_setproperty(J, -2, "shift");
	js_newcfunction(J, jsB_ReadStream_prototype_end, "end", 0);
	js_setproperty(J, -2, "end");
	js_newcfunction(J, jsB_ReadStream_prototype_close, "close", 0);
	js_setproperty(J, -2, "close");
	js_newcfunction(J, jsB_ReadStream_prototype_addEventListener, "addEventListener", 2);
	js_setproperty(J, -2, "addEventListener");
	js_newcfunction(J, jsB_ReadStream_prototype_removeEventListener, "removeEventListener", 2);
	js_setproperty(J, -2, "removeEventListener");
	js_newcconstructor(J, jsB_call_ReadStream, jsB_new_ReadStream, "ReadStream", 0);

	js_copy(J, -1);
	js_setregistry(J, "jsB_ReadStream");
}
