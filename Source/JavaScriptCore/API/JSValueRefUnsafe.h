/*
 * Copyright (C) 2015 Swise Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Swise Corporation nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SWISE CORPORATION AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Swise Corporation OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JSValueRefUnsafe_h
#define JSValueRefUnsafe_h

#include <JavaScriptCore/JSValueRef.h>

typedef struct OpaqueJSHandle* JSHandleRef;

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
@function
@abstract       Converts a JavaScript value to number and returns the resulting number.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         The numeric result of conversion, or NaN if an exception is thrown.
*/
JS_EXPORT int32_t JSValueToIntNumber(JSContextRef ctx, JSValueRef value, JSValueRef* exception);

/* JSLR additions*/

/*!
@function
@abstract       Returns a JavaScript value's type.
@param ctx  The execution context to use.
@param value    The JSValue whose type you want to obtain.
@result         A value of type JSType that identifies value's type.
*/
JS_EXPORT ::JSType JSParameterGetType(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the undefined type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the undefined type, otherwise false.
*/
JS_EXPORT bool JSParameterIsUndefined(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the null type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the null type, otherwise false.
*/
JS_EXPORT bool JSParameterIsNull(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the boolean type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the boolean type, otherwise false.
*/
JS_EXPORT bool JSParameterIsBoolean(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the boolean type and returns value.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the boolean type, otherwise false.
*/
JS_EXPORT bool JSTryGetParameterAsBoolean(JSContextRef ctx, JSValueRef value, bool* result);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the number type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the number type, otherwise false.
*/
JS_EXPORT bool JSParameterIsNumber(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the number type and returns value.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the number type, otherwise false.
*/
JS_EXPORT bool JSTryGetParameterAsNumber(JSContextRef ctx, JSValueRef value, double* result);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the string type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the string type, otherwise false.
*/
JS_EXPORT bool JSParameterIsString(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Tests whether a JavaScript value's type is the object type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the object type, otherwise false.
*/
JS_EXPORT bool JSParameterIsObject(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract Tests whether a JavaScript value is an object with a given class in its class chain.
@param ctx The execution context to use.
@param value The JSValue to test.
@param jsClass The JSClass to test against.
@result true if value is an object and has jsClass in its class chain, otherwise false.
*/
JS_EXPORT bool JSParameterIsObjectOfClass(JSContextRef ctx, JSValueRef value, JSClassRef jsClass);

/*!
@function
@abstract Tests whether two JavaScript values are equal, as compared by the JS == operator.
@param ctx The execution context to use.
@param a The first value to test.
@param b The second value to test.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result true if the two values are equal, false if they are not equal or an exception is thrown.
*/
JS_EXPORT bool JSParameterIsEqual(JSContextRef ctx, JSValueRef a, JSValueRef b, JSValueRef* exception);

/*!
@function
@abstract       Tests whether two JavaScript values are strict equal, as compared by the JS === operator.
@param ctx  The execution context to use.
@param a        The first value to test.
@param b        The second value to test.
@result         true if the two values are strict equal, otherwise false.
*/
JS_EXPORT bool JSParameterIsStrictEqual(JSContextRef ctx, JSValueRef a, JSValueRef b);

/*!
@function
@abstract Tests whether a JavaScript value is an object constructed by a given constructor, as compared by the JS instanceof operator.
@param ctx The execution context to use.
@param value The JSValue to test.
@param constructor The constructor to test against.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result true if value is an object constructed by constructor, as compared by the JS instanceof operator, otherwise false.
*/
JS_EXPORT bool JSParameterIsInstanceOfConstructor(JSContextRef ctx, JSValueRef value, JSObjectRef constructor, JSValueRef* exception);

/*!
@function
@abstract       Converts a JavaScript value to boolean and returns the resulting boolean.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@result         The boolean result of conversion.
*/
JS_EXPORT bool JSValueToBooleanUnsafe(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Converts a JavaScript value to number and returns the resulting number.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         The numeric result of conversion, or NaN if an exception is thrown.
*/
JS_EXPORT double JSValueToNumberUnsafe(JSContextRef ctx, JSValueRef value, JSValueRef* exception);

/*!
@function
@abstract       Converts a JavaScript value to number and returns the resulting number.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         The numeric result of conversion, or NaN if an exception is thrown.
*/
JS_EXPORT int32_t JSValueToIntNumberUnsafe(JSContextRef ctx, JSValueRef value, JSValueRef* exception);

/*!
@function
@abstract       Converts a JavaScript value to string and copies the result into a JavaScript string.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         A JSString with the result of conversion, or NULL if an exception is thrown. Ownership follows the Create Rule.
*/
JS_EXPORT JSStringRef JSValueToStringCopyUnsafe(JSContextRef ctx, JSValueRef value, JSValueRef* exception);

/*!
@function
@abstract       Creates a JavaScript value of the undefined type.
@param ctx  The execution context to use.
@result         The unique undefined value.
*/
JS_EXPORT JSValueRef JSValueMakeUndefinedUnsafe(JSContextRef ctx);

/*!
@function
@abstract       Creates a JavaScript value of the null type.
@param ctx  The execution context to use.
@result         The unique null value.
*/
JS_EXPORT JSValueRef JSValueMakeNullUnsafe(JSContextRef ctx);

/*!
@function
@abstract       Creates a JavaScript value of the boolean type.
@param ctx  The execution context to use.
@param boolean  The bool to assign to the newly created JSValue.
@result         A JSValue of the boolean type, representing the value of boolean.
*/
JS_EXPORT JSValueRef JSValueMakeBooleanUnsafe(JSContextRef ctx, bool boolean);

/*!
@function
@abstract       Creates a JavaScript value of the number type.
@param ctx  The execution context to use.
@param number   The double to assign to the newly created JSValue.
@result         A JSValue of the number type, representing the value of number.
*/
JS_EXPORT JSValueRef JSValueMakeNumberUnsafe(JSContextRef ctx, double number);

/*!
@function
@abstract       Creates a JavaScript value of the string type.
@param ctx  The execution context to use.
@param string   The JSString to assign to the newly created JSValue. The
newly created JSValue retains string, and releases it upon garbage collection.
@result         A JSValue of the string type, representing the value of string.
*/
JS_EXPORT JSValueRef JSValueMakeStringUnsafe(JSContextRef ctx, JSStringRef string);

/* Converting to and from JSON formatted strings */

/*!
@function
@abstract       Creates a JavaScript value from a JSON formatted string.
@param ctx      The execution context to use.
@param string   The JSString containing the JSON string to be parsed.
@result         A JSValue containing the parsed value, or NULL if the input is invalid.
*/
JS_EXPORT JSValueRef JSValueMakeFromJSONStringUnsafe(JSContextRef ctx, JSStringRef string);

/*!
@function
@abstract       Creates a JavaScript string containing the JSON serialized representation of a JS value.
@param ctx      The execution context to use.
@param value    The value to serialize.
@param indent   The number of spaces to indent when nesting.  If 0, the resulting JSON will not contains newlines.  The size of the indent is clamped to 10 spaces.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         A JSString with the result of serialization, or NULL if an exception is thrown.
*/
JS_EXPORT JSStringRef JSValueCreateJSONStringUnsafe(JSContextRef ctx, JSValueRef value, unsigned indent, JSValueRef* exception);

/* Garbage collection */
/*!
@function
@abstract Protects a JavaScript value from garbage collection.
@param ctx The execution context to use.
@param value The JSValue to protect.
@discussion Use this method when you want to store a JSValue in a global or on the heap, where the garbage collector will not be able to discover your reference to it.

A value may be protected multiple times and must be unprotected an equal number of times before becoming eligible for garbage collection.
*/
JS_EXPORT JSHandleRef JSFunctionProtect(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract       Unprotects a JavaScript value from garbage collection.
@param ctx      The execution context to use.
@param value    The JSValue to unprotect.
@discussion     A value may be protected multiple times and must be unprotected an
equal number of times before becoming eligible for garbage collection.
*/
JS_EXPORT void JSFunctionUnprotect(JSContextRef ctx, JSHandleRef handle);

#ifdef __cplusplus
}
#endif

#endif /* JSValueRef_h */
