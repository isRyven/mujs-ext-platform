#include <stdlib.h>
#include <string.h>
#include "jsB_arraybuffer.h"
#include "jsB_utils.h"

char str_is_number(const char *text)
{
    int j = strlen(text);
    while(j--) {
        if(text[j] >= '0' && text[j] <= '9') {
            continue;
        }
        return 0;
    }
    return 1;
}

void jsB_toString(js_State *J)
{
	enum enc_t {
		AUTO,
		ASCII,
		UTF8
	};
	if (js_isundefined(J, 1)) {
		js_pushconst(J, "");
		return;	
	}
	/* get encoding */
	enum enc_t encoding = AUTO;
	if (js_isstring(J, 2)) {
		const char *encArg = js_tostring(J, 2);
		if (strcmp(encArg, "ascii") == 0) {
			encoding = ASCII;
		} else if (strcmp(encArg, "utf8") == 0) {
			encoding = UTF8;
		} else {
			js_typeerror(J, 
				"unsupported encoding: '%s', valid encodings are: 'ascii', 'utf8'", encArg);
		}
	}
	/* get data */
	const char *data = NULL;
	int size = 0;
	if (jsB_ArrayBuffer_instance(J, 1)) {
		js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
		data = backStore->data;
		size = backStore->length;
	} else {
		data = js_tostring(J, 1);
		size = js_getstrsize(J, 1);
	}
	/* make string */
	switch (encoding) {
		case ASCII:
			js_pushlstringu(J, data, size, 0);
			break;
		case UTF8:
			js_pushlstringu(J, data, size, 1);
			break;
		default:
			js_pushlstring(J, data, size);
			break;
	}
}

void jsB_toArrayBuffer(js_State *J)
{
	if (jsB_ArrayBuffer_instance(J, 1)) {
		js_copy(J, 1);
		return;
	} else if (js_isarray(J, 1)) {
		js_getlength(J, 1);
		int length = js_tonumber(J, 1);
		jsB_ArrayBuffer(J);
		js_pushnumber(J, length);
		js_construct(J, 1);
		js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
		for (int i = 0; i < length; ++i) {
			js_getindex(J, 1, i);
			unsigned char element = (unsigned char)js_checkinteger(J, -1, 0);
			backStore->data[i] = element;
			js_pop(J, 1);
		}
	} else if (js_isobject(J, 1)) {
		if (!js_hasproperty(J, 1, "length")) {
			js_typeerror(J, "expected object to have length property");
		}
		int length = js_tointeger(J, -1);
		js_pop(J, 1);
		jsB_ArrayBuffer(J);
		js_pushnumber(J, length);
		js_construct(J, 1);
		js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
		js_pushiterator(J, 1, 1);
		const char *propName;
		while ((propName = js_nextiterator(J, -1))) {
			if (!str_is_number(propName)) {
				continue;
			}
			int i = atoi(propName);
			if (i < 0 || i >= length ) {
				continue;
			}
			js_getproperty(J, 1, propName);
			unsigned char element = (unsigned char)js_checkinteger(J, -1, 0);
			backStore->data[i] = element;
			js_pop(J, 1);
		}
		js_pop(J, 1);
	} else {
		const char *str = js_trystring(J, 1, "");
		int size = strlen(str);
		jsB_ArrayBuffer(J);
		js_pushnumber(J, size);
		js_construct(J, 1);
		js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
		memcpy(backStore->data, str, size);
	}
}

void jsB_Utils(js_State *J)
{
	js_getregistry(J, "jsB_Utils");
	if (js_isdefined(J, -1)) {
		return;
	} 
	js_pop(J, 1);
	/* namespace */
	js_newobject(J);
	js_newcfunction(J, jsB_toString, "toString", 2);
	js_defproperty(J, -2, "toString", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_toArrayBuffer, "toArrayBuffer", 1);
	js_defproperty(J, -2, "toArrayBuffer", JS_READONLY | JS_DONTCONF);
	/* cache */
	js_copy(J, -1);
	js_setregistry(J, "jsB_Utils");
}
