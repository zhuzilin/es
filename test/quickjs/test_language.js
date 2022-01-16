function assert(actual, expected, message) {
    if (arguments.length == 1)
        expected = true;

    if (actual === expected)
        return;

    if (actual !== null && expected !== null
    &&  typeof actual == 'object' && typeof expected == 'object'
    &&  actual.toString() === expected.toString())
        return;

        "assertion failed: got |" + actual
      console_log("before throw")

    throw Error("assertion failed: got |" + actual + "|" +
                ", expected |" + expected + "|" +
                (message ? " (" + message + ")" : ""));
}

function assert_throws(expected_error, func)
{
    var err = false;
    try {
        func();
    } catch(e) {
        err = true;
        if (!(e instanceof expected_error)) {
            throw Error("unexpected exception type");
        }
    }
    if (!err) {
        throw Error("expected exception");
    }
}

// load more elaborate version of assert if available
try { __loadScript("test_assert.js"); } catch(e) {}

/*----------------*/

function test_op1()
{
    var r, a;
    r = 1 + 2;
    assert(r, 3, "1 + 2 === 3");

    r = 1 - 2;
    assert(r, -1, "1 - 2 === -1");

    r = -1;
    assert(r, -1, "-1 === -1");

    r = +2;
    assert(r, 2, "+2 === 2");

    r = 2 * 3;
    assert(r, 6, "2 * 3 === 6");

    r = 4 / 2;
    assert(r, 2, "4 / 2 === 2");

    r = 4 % 3;
    assert(r, 1, "4 % 3 === 3");

    r = 4 << 2;
    assert(r, 16, "4 << 2 === 16");

    r = 1 << 0;
    assert(r, 1, "1 << 0 === 1");

    r = 1 << 31;
    assert(r, -2147483648, "1 << 31 === -2147483648");
    
    r = 1 << 32;
    assert(r, 1, "1 << 32 === 1");
    
    r = (1 << 31) < 0;
    assert(r, true, "(1 << 31) < 0 === true");

    r = -4 >> 1;
    assert(r, -2, "-4 >> 1 === -2");

    r = -4 >>> 1;
    assert(r, 0x7ffffffe, "-4 >>> 1 === 0x7ffffffe");

    r = 1 & 1;
    assert(r, 1, "1 & 1 === 1");

    r = 0 | 1;
    assert(r, 1, "0 | 1 === 1");

    r = 1 ^ 1;
    assert(r, 0, "1 ^ 1 === 0");

    r = ~1;
    assert(r, -2, "~1 === -2");

    r = !1;
    assert(r, false, "!1 === false");

    assert((1 < 2), true, "(1 < 2) === true");

    assert((2 > 1), true, "(2 > 1) === true");

    assert(('b' > 'a'), true, "('b' > 'a') === true");
}

function test_cvt()
{
    assert((NaN | 0) === 0);
    assert((Infinity | 0) === 0);
    assert(((-Infinity) | 0) === 0);
    assert(("12345" | 0) === 12345);
    assert(("0x12345" | 0) === 0x12345);
    assert(((4294967296 * 3 - 4) | 0) === -4);
    
    assert(("12345" >>> 0) === 12345);
    assert(("0x12345" >>> 0) === 0x12345);
    assert((NaN >>> 0) === 0);
    assert((Infinity >>> 0) === 0);
    assert(((-Infinity) >>> 0) === 0);
    assert(((4294967296 * 3 - 4) >>> 0) === (4294967296 - 4));
}

function test_eq()
{
    assert(null == undefined);
    assert(undefined == null);
    assert(true == 1);
    assert(0 == false);
    assert("" == 0);
    assert("123" == 123);
    assert("122" != 123);
    assert((new Number(1)) == 1);
    assert(2 == (new Number(2)));
    // assert((new String("abc")) == "abc");
    assert({} != "abc");
}

function test_inc_dec()
{
    var a, r;
    
    a = 1;
    r = a++;
    assert(r === 1 && a === 2, true, "++");

    a = 1;
    r = ++a;
    assert(r === 2 && a === 2, true, "++");

    a = 1;
    r = a--;
    assert(r === 1 && a === 0, true, "--");

    a = 1;
    r = --a;
    assert(r === 0 && a === 0, true, "--");

    a = {x:true};
    a.x++;
    assert(a.x, 2, "++");

    a = {x:true};
    a.x--;
    assert(a.x, 0, "--");

    a = [true];
    a[0]++;
    assert(a[0], 2, "++");
    
    a = {x:true};
    r = a.x++;
    assert(r === 1 && a.x === 2, true, "++");
    
    a = {x:true};
    r = a.x--;
    assert(r === 1 && a.x === 0, true, "--");
    
    a = [true];
    r = a[0]++;
    assert(r === 1 && a[0] === 2, true, "++");
    
    a = [true];
    r = a[0]--;
    assert(r === 1 && a[0] === 0, true, "--");
}

function F(x)
{
    this.x = x;
}

function test_op2()
{
    var a, b;
    a = new Object;
    a.x = 1;
    assert(a.x, 1, "new");
    b = new F(2);
    assert(b.x, 2, "new");

    a = {x : 2};
    assert(("x" in a), true, "in");
    assert(("y" in a), false, "in");

    // a = {};
    // assert((a instanceof Object), true, "instanceof");
    // assert((a instanceof String), false, "instanceof");

    assert((typeof 1), "number", "typeof");
    assert((typeof Object), "function", "typeof");
    assert((typeof null), "object", "typeof");
    assert((typeof unknown_var), "undefined", "typeof");
    
    a = {x: 1, if: 2, async: 3};
    assert(a.if === 2);
    assert(a.async === 3);
}

function test_delete()
{
    var a, err;

    a = {x: 1, y: 1};
    assert((delete a.x), true, "delete");
    assert(("x" in a), false, "delete");
    
    // /* the following are not tested by test262 */
    // assert(delete "abc"[100], true);

    // err = false;
    // try {
    //     delete null.a;
    // } catch(e) {
    //     err = (e instanceof TypeError);
    // }
    // assert(err, true, "delete");
}

function test_prototype()
{
    var f = function f() { };
    console_log(f.prototype.toString())
    console_log(f.prototype.constructor.toString());
    assert(f.prototype.constructor, f, "prototype");
}

function test_arguments()
{
    function f2() {
        assert(arguments.length, 2, "arguments");
        assert(arguments[0], 1, "arguments");
        assert(arguments[1], 3, "arguments");
    }
    f2(1, 3);
}

function test_object_literal()
{
    a = { get: 2, set: 3, async: 4 };
    assert(JSON.stringify(a), '{"get":2,"set":3,"async":4}');
}

function test_regexp_skip()
{
    var a, b;
    [a, b = /abc\(/] = [1];
    assert(a === 1);
    
    [a, b =/abc\(/] = [2];
    assert(a === 2);
}

function test_labels()
{
    do x: { break x; } while(0);
    if (1)
        x: { break x; }
    else
        x: { break x; }
    with ({}) x: { break x; };
    while (0) x: { break x; };
}

function test_function_expr_name()
{
    var f;

    /* non strict mode test : assignment to the function name silently
       fails */
    
    f = function myfunc() {
        myfunc = 1;
        return myfunc;
    };
    assert(f(), f);

    f = function myfunc() {
        eval("myfunc = 1");
        return myfunc;
    };
    assert(f(), f);
    
    /* strict mode test : assignment to the function name raises a
       TypeError exception */

    f = function myfunc() {
        "use strict";
        myfunc = 1;
    };
    assert_throws(TypeError, f);

    f = function myfunc() {
        "use strict";
        eval("myfunc = 1");
    };
    assert_throws(TypeError, f);
}

test_op1();
test_cvt();
test_eq();
test_inc_dec();
test_op2();
test_delete();
test_prototype();
// test_arguments();
// test_object_literal();
// test_regexp_skip();
// test_labels();
// test_function_expr_name();
