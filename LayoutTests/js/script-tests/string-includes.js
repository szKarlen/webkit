description("This test checks the ES6 string functions startsWith(), endsWith() and includes().");

// Test includes
shouldBe("'foo bar'.includes('bar')", "true");
shouldBe("'foo bar'.includes('bar', 4)", "true");
shouldBe("'foo bar'.includes('ar', 5)", "true");
shouldBe("'foo bar'.includes('qux')", "false");
shouldBe("'foo bar'.includes('foo')", "true");
shouldBe("'foo bar'.includes('foo', 0)", "true");
shouldBe("'foo bar'.includes('foo', -1)", "true");
shouldBe("'foo bar'.includes('')", "true");
shouldBe("'foo bar'.includes()", "false");
shouldBe("'foo bar qux'.includes('qux', 7)", "true");
shouldBe("'foo bar qux'.includes('bar', 7)", "false");
shouldBe("'foo null bar'.includes()", "false");
shouldBe("'foo null bar'.includes(null)", "true");
shouldBe("'foo null bar'.includes(null)", "true");
shouldBe("'foo undefined bar'.includes()", "true");
shouldBe("'foo undefined bar'.includes(undefined)", "true");
shouldBe("'foo undefined bar'.includes()", "true");
shouldBe("'foo undefined bar'.includes()", "true");
shouldBe("'foo true bar'.includes(true)", "true");
shouldBe("'foo false bar'.includes(false)", "true");
shouldBe("'foo 1 bar'.includes(1)", "true");
shouldBe("'foo 1.1 bar'.includes(1.1)", "true");
shouldBe("'foo NaN bar'.includes(NaN)", "true");
shouldBe("'foo 1.0 bar'.includes(1.0)", "true");
shouldBe("'foo 1e+100 bar'.includes(1e+100)", "true");
shouldBe("'foo 1e100 bar'.includes(1e100)", "false");
shouldBe("'フーバー'.includes('ーバ')", "true");
shouldBe("'フーバー'.includes('クー')", "false");

// Test startsWith
shouldBe("'foo bar'.startsWith('foo')", "true");
shouldBe("'foo bar'.startsWith('foo', 0)", "true");
shouldBe("'foo bar'.startsWith('foo', -1)", "true");
shouldBe("'foo bar'.startsWith('oo', 1)", "true");
shouldBe("'foo bar'.startsWith('qux')", "false");
shouldBe("'foo bar'.startsWith('')", "true");
shouldBe("'foo bar'.startsWith()", "false");
shouldBe("'null'.startsWith()", "false");
shouldBe("'null'.startsWith(null)", "true");
shouldBe("'null bar'.startsWith(null)", "true");
shouldBe("'undefined'.startsWith()", "true");
shouldBe("'undefined'.startsWith(undefined)", "true");
shouldBe("'undefined bar'.startsWith()", "true");
shouldBe("'undefined bar'.startsWith()", "true");
shouldBe("'true bar'.startsWith(true)", "true");
shouldBe("'false bar'.startsWith(false)", "true");
shouldBe("'1 bar'.startsWith(1)", "true");
shouldBe("'1.1 bar'.startsWith(1.1)", "true");
shouldBe("'NaN bar'.startsWith(NaN)", "true");
shouldBe("'1e+100 bar'.startsWith(1e+100)", "true");
shouldBe("'1e100 bar'.startsWith(1e100)", "false");
shouldBe("'フーバー'.startsWith('フー')", "true");
shouldBe("'フーバー'.startsWith('バー')", "false");

// Test endsWith
shouldBe("'foo bar'.endsWith('bar')", "true");
shouldBe("'foo bar'.endsWith('ba', 6)", "true");
shouldBe("'foo bar'.endsWith(' ba', 6)", "true");
shouldBe("'foo bar'.endsWith('foo bar')", "true");
shouldBe("'foo bar'.endsWith('foo bar', 7)", "true");
shouldBe("'foo bar'.endsWith('foo bar', 8)", "true");
shouldBe("'foo bar'.endsWith('foo bar', -1)", "false");
shouldBe("'foo bar'.endsWith('qux')", "false");
shouldBe("'foo bar'.endsWith('')", "true");
shouldBe("'foo bar'.endsWith()", "false");
shouldBe("'foo null'.endsWith()", "false");
shouldBe("'foo null'.endsWith(null)", "true");
shouldBe("'foo null'.endsWith(null)", "true");
shouldBe("'foo undefined'.endsWith()", "true");
shouldBe("'foo undefined'.endsWith(undefined)", "true");
shouldBe("'foo undefined'.endsWith()", "true");
shouldBe("'foo undefined'.endsWith()", "true");
shouldBe("'foo true'.endsWith(true)", "true");
shouldBe("'foo false'.endsWith(false)", "true");
shouldBe("'foo 1'.endsWith(1)", "true");
shouldBe("'foo 1.1'.endsWith(1.1)", "true");
shouldBe("'foo NaN'.endsWith(NaN)", "true");
shouldBe("'foo 1e+100'.endsWith(1e+100)", "true");
shouldBe("'foo 1e100'.endsWith(1e100)", "false");
shouldBe("'フーバー'.endsWith('バー')", "true");
shouldBe("'フーバー'.endsWith('フー')", "false");

// Call functions with an environment record as 'this'.
shouldThrow("(function() { var f = String.prototype.startsWith; (function() { f('a'); })(); })()");
shouldThrow("(function() { var f = String.prototype.endsWith; (function() { f('a'); })(); })()");
shouldThrow("(function() { var f = String.prototype.includes; (function() { f('a'); })(); })()");

// ES6 spec says a regex as argument should throw an Exception.
shouldThrow("'foo bar'.startsWith(/\w+/)");
shouldThrow("'foo bar'.endsWith(/\w+/)");
shouldThrow("'foo bar'.includes(/\w+/)");

// Check side effects in startsWith.
var sideEffect = "";
var stringToSearchIn = new String("foo bar");
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
var startOffset = new Number(0);
startOffset.valueOf = function() {
    sideEffect += "B";
    return this;
}
var matchString = new String("foo");
matchString.toString = function() {
    sideEffect += "C";
    return this;
}
// Calling stringToSearchIn.startsWith implicitly calls stringToSearchIn.toString(),
// startOffset.valueOf() and matchString.toString(), in that respective order.
shouldBe("stringToSearchIn.startsWith(matchString, startOffset)", "true");
shouldBe("sideEffect == 'ABC'", "true");

// If stringToSearchIn throws an exception startOffset.valueOf() and
// matchString.toString() are not called.
stringToSearchIn.toString = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.startsWith(matchString, startOffset)", "'error'");
shouldBe("sideEffect == ''", "true");

// If startOffset throws an exception stringToSearchIn.toString() is called but
// matchString.toString() is not.
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
startOffset.valueOf = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.startsWith(matchString, startOffset)", "'error'");
shouldBe("sideEffect == 'A'", "true");

// If matchString.toString() throws an exception stringToSearchIn.toString() and
// startOffset.valueOf() were called.
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
startOffset.valueOf = function() {
    sideEffect += "B";
    return this;
}
matchString.toString = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.startsWith(matchString, startOffset)", "'error'");
shouldBe("sideEffect == 'AB'", "true");

// Check side effects in endsWith.
sideEffect = "";
stringToSearchIn = new String('foo bar');
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
var endOffset = new Number(stringToSearchIn.length);
endOffset.valueOf = function() {
    sideEffect += "B";
    return this;
}
matchString = new String('bar');
matchString.toString = function() {
    sideEffect += "C";
    return this;
}

// Calling stringToSearchIn.endsWith implicitly calls stringToSearchIn.toString(),
// endOffset.valueOf() and matchString.toString(), in that respective order.
shouldBe("stringToSearchIn.endsWith(matchString, endOffset)", "true");
shouldBe("sideEffect == 'ABC'", "true");

// If stringToSearchIn throws an exception endOffset.valueOf() and
// matchString.toString() are not called.
stringToSearchIn.toString = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.endsWith(matchString, endOffset)", "'error'");
shouldBe("sideEffect == ''", "true");

// If endOffset throws an exception stringToSearchIn.toString() is called but
// matchString.toString() is not.
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
endOffset.valueOf = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.endsWith(matchString, endOffset)", "'error'");
shouldBe("sideEffect == 'A'", "true");

// If matchString.toString() throws an exception stringToSearchIn.toString() and
// endOffset.valueOf() were called.
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
endOffset.valueOf = function() {
    sideEffect += "B";
    return this;
}
matchString.toString = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.endsWith(matchString, endOffset)", "'error'");
shouldBe("sideEffect == 'AB'", "true");

// Check side effects in includes.
var sideEffect = "";
stringToSearchIn = new String("foo bar");
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
var startOffset = new Number(0);
startOffset.valueOf = function() {
    sideEffect += "B";
    return this;
}
matchString = new String("foo");
matchString.toString = function() {
    sideEffect += "C";
    return this;
}
// Calling stringToSearchIn.includes implicitly calls stringToSearchIn.toString(),
// startOffset.valueOf() and matchString.toString(), in that respective order.
shouldBe("stringToSearchIn.includes(matchString, startOffset)", "true");
shouldBe("sideEffect == 'ABC'", "true");

// If stringToSearchIn throws an exception startOffset.valueOf() and
// matchString.toString() are not called.
stringToSearchIn.toString = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.includes(matchString, startOffset)", "'error'");
shouldBe("sideEffect == ''", "true");

// If startOffset throws an exception stringToSearchIn.toString() is called but
// matchString.toString() is not.
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
startOffset.valueOf = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.includes(matchString, startOffset)", "'error'");
shouldBe("sideEffect == 'A'", "true");

// If matchString.toString() throws an exception stringToSearchIn.toString() and
// startOffset.valueOf() were called.
stringToSearchIn.toString = function() {
    sideEffect += "A";
    return this;
}
startOffset.valueOf = function() {
    sideEffect += "B";
    return this;
}
matchString.toString = function() {
    throw "error";
}
sideEffect = "";
shouldThrow("stringToSearchIn.includes(matchString, startOffset)", "'error'");
shouldBe("sideEffect == 'AB'", "true");
