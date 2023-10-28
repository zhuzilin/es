/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.

//-----------------------------------------------------------------------------
function compareArray(aExpected, aActual) {
  if (aActual.length != aExpected.length) {
      return false;
  }

  aExpected.sort();
  aActual.sort();

  var s;
  for (var i = 0; i < aExpected.length; i++) {
      if (aActual[i] !== aExpected[i]) {
          return false;
      }
  }
  return true;
}

//-----------------------------------------------------------------------------
function arrayContains(arr, expected) {
  var found;
  for (var i = 0; i < expected.length; i++) {
      found = false;
      for (var j = 0; j < arr.length; j++) {
          if (expected[i] === arr[j]) {
              found = true;
              break;
          }
      }
      if (!found) {
          return false;
      }
  }
  return true;
}

//-----------------------------------------------------------------------------
var supportsArrayIndexGettersOnArrays = undefined;
function fnSupportsArrayIndexGettersOnArrays() {
  if (typeof supportsArrayIndexGettersOnArrays !== "undefined") {
      return supportsArrayIndexGettersOnArrays;
  }

  supportsArrayIndexGettersOnArrays = false;

  if (fnExists(Object.defineProperty)) {
      var arr = [];
      Object.defineProperty(arr, "0", {
          get: function() {
              supportsArrayIndexGettersOnArrays = true;
              return 0;
          }
      });
      var res = arr[0];
  }

  return supportsArrayIndexGettersOnArrays;
}

//-----------------------------------------------------------------------------
var supportsArrayIndexGettersOnObjects = undefined;
function fnSupportsArrayIndexGettersOnObjects() {
  if (typeof supportsArrayIndexGettersOnObjects !== "undefined")
      return supportsArrayIndexGettersOnObjects;

  supportsArrayIndexGettersOnObjects = false;

  if (fnExists(Object.defineProperty)) {
      var obj = {};
      Object.defineProperty(obj, "0", {
          get: function() {
              supportsArrayIndexGettersOnObjects = true;
              return 0;
          }
      });
      var res = obj[0];
  }

  return supportsArrayIndexGettersOnObjects;
}

//-----------------------------------------------------------------------------
function ConvertToFileUrl(pathStr) {
  return "file:" + pathStr.replace(/\\/g, "/");
}

//-----------------------------------------------------------------------------
function fnExists(/*arguments*/) {
  for (var i = 0; i < arguments.length; i++) {
      if (typeof (arguments[i]) !== "function") return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
var __globalObject = Function("return this;")();
function fnGlobalObject() {
   return __globalObject;
}

//-----------------------------------------------------------------------------
function fnSupportsStrict() {
  "use strict";
  try {
      eval('with ({}) {}');
      return false;
  } catch (e) {
      return true;
  }
}

//-----------------------------------------------------------------------------
//Verify all attributes specified data property of given object:
//value, writable, enumerable, configurable
//If all attribute values are expected, return true, otherwise, return false
function dataPropertyAttributesAreCorrect(obj,
                                        name,
                                        value,
                                        writable,
                                        enumerable,
                                        configurable) {
  var attributesCorrect = true;

  if (obj[name] !== value) {
      if (typeof obj[name] === "number" &&
          isNaN(obj[name]) &&
          typeof value === "number" &&
          isNaN(value)) {
          // keep empty
      } else {
          attributesCorrect = false;
      }
  }

  try {
      if (obj[name] === "oldValue") {
          obj[name] = "newValue";
      } else {
          obj[name] = "OldValue";
      }
  } catch (we) {
  }

  var overwrited = false;
  if (obj[name] !== value) {
      if (typeof obj[name] === "number" &&
          isNaN(obj[name]) &&
          typeof value === "number" &&
          isNaN(value)) {
          // keep empty
      } else {
          overwrited = true;
      }
  }
  if (overwrited !== writable) {
      attributesCorrect = false;
  }

  var enumerated = false;
  for (var prop in obj) {
      if (obj.hasOwnProperty(prop) && prop === name) {
          enumerated = true;
      }
  }

  if (enumerated !== enumerable) {
      attributesCorrect = false;
  }


  var deleted = false;

  try {
      delete obj[name];
  } catch (de) {
  }
  if (!obj.hasOwnProperty(name)) {
      deleted = true;
  }
  if (deleted !== configurable) {
      attributesCorrect = false;
  }

  return attributesCorrect;
}

//-----------------------------------------------------------------------------
//Verify all attributes specified accessor property of given object:
//get, set, enumerable, configurable
//If all attribute values are expected, return true, otherwise, return false
function accessorPropertyAttributesAreCorrect(obj,
                                            name,
                                            get,
                                            set,
                                            setVerifyHelpProp,
                                            enumerable,
                                            configurable) {
  var attributesCorrect = true;

  if (get !== undefined) {
      if (obj[name] !== get()) {
          if (typeof obj[name] === "number" &&
              isNaN(obj[name]) &&
              typeof get() === "number" &&
              isNaN(get())) {
              // keep empty
          } else {
              attributesCorrect = false;
          }
      }
  } else {
      if (obj[name] !== undefined) {
          attributesCorrect = false;
      }
  }

  try {
      var desc = Object.getOwnPropertyDescriptor(obj, name);
      if (typeof desc.set === "undefined") {
          if (typeof set !== "undefined") {
              attributesCorrect = false;
          }
      } else {
          obj[name] = "toBeSetValue";
          if (obj[setVerifyHelpProp] !== "toBeSetValue") {
              attributesCorrect = false;
          }
      }
  } catch (se) {
      throw se;
  }


  var enumerated = false;
  for (var prop in obj) {
      if (obj.hasOwnProperty(prop) && prop === name) {
          enumerated = true;
      }
  }

  if (enumerated !== enumerable) {
      attributesCorrect = false;
  }


  var deleted = false;
  try {
      delete obj[name];
  } catch (de) {
      throw de;
  }
  if (!obj.hasOwnProperty(name)) {
      deleted = true;
  }
  if (deleted !== configurable) {
      attributesCorrect = false;
  }

  return attributesCorrect;
}

//-----------------------------------------------------------------------------
var NotEarlyErrorString = "NotEarlyError";
var EarlyErrorRePat = "^((?!" + NotEarlyErrorString + ").)*$";
var NotEarlyError = new Error(NotEarlyErrorString);

//-----------------------------------------------------------------------------
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

function Test262Error(message) {
  if (message) this.message = message;
}

Test262Error.prototype.toString = function () {
  return "Test262 Error: " + this.message;
};

function testFailed(message) {
  throw new Test262Error(message);
}


function testPrint(message) {

}


//adaptors for Test262 framework
function $PRINT(message) {

}

function $INCLUDE(message) { }
function $ERROR(message) {
  testFailed(message);
}

function $FAIL(message) {
  testFailed(message);
}



//Sputnik library definitions
//Ultimately these should be namespaced some how and only made
//available to tests that explicitly include them.
//For now, we just define the globally

//math_precision.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

function getPrecision(num) {
  //TODO: Create a table of prec's,
  //      because using Math for testing Math isn't that correct.

  var log2num = Math.log(Math.abs(num)) / Math.LN2;
  var pernum = Math.ceil(log2num);
  return (2 * Math.pow(2, -52 + pernum));
  //return(0);
}


//math_isequal.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

var prec;
function isEqual(num1, num2) {
  if ((num1 === Infinity) && (num2 === Infinity)) {
      return (true);
  }
  if ((num1 === -Infinity) && (num2 === -Infinity)) {
      return (true);
  }
  prec = getPrecision(Math.min(Math.abs(num1), Math.abs(num2)));
  return (Math.abs(num1 - num2) <= prec);
  //return(num1 === num2);
}

//numeric_conversion.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

function ToInteger(p) {
  var x = Number(p);

  if (isNaN(x)) {
      return +0;
  }

  if ((x === +0)
|| (x === -0)
|| (x === Number.POSITIVE_INFINITY)
|| (x === Number.NEGATIVE_INFINITY)) {
      return x;
  }

  var sign = (x < 0) ? -1 : 1;

  return (sign * Math.floor(Math.abs(x)));
}

//--Test case registration-----------------------------------------------------
function runTestCase(testcase) {
  if (testcase() !== true) {
      $ERROR("Test case returned non-true value!");
  }
}
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * Variable instantiation is performed using the global object as
 * the variable object and using property attributes { DontDelete }
 *
 * @path ch10/10.4/10.4.1/S10.4.1_A1_T1.js
 * @description Checking if deleting variable x, that is defined as var x = 1, fails
 * @noStrict
 */

var x = 1;

if (this.x !== 1) {
  $ERROR("#1: variable x is a property of global object");
}

if(delete this.x !== false){
  $ERROR("#2: variable x has property attribute DontDelete");
}

