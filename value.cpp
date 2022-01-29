
#include "es.h"

namespace es
{

    JSObject* ToObject(Error* e, JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
            case JSValue::JS_NULL:
                *e = *Error::TypeError("Cannot convert undefined or null to object");
                return nullptr;
            case JSValue::JS_BOOL:
                return new BoolObject(input);
            case JSValue::JS_NUMBER:
                return new NumberObject(input);
            case JSValue::JS_STRING:
                return new StringObject(input);
            case JSValue::JS_OBJECT:
                return static_cast<JSObject*>(input);
            default:
                assert(false);
        }

    }// namespace es

    // 11.8.5 The Abstract Relational Comparison Algorithm
    // x < y
    JSValue* LessThan(Error* e, JSValue* x, JSValue* y, bool left_first)
    {
        JSValue* px;
        JSValue* py;
        if(left_first)
        {
            px = ToPrimitive(e, x, "Number");
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
            py = ToPrimitive(e, y, "Number");
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
        }
        else
        {
            py = ToPrimitive(e, y, "Number");
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
            px = ToPrimitive(e, x, "Number");
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
        }
        if(!px->IsString() && !py->IsString())
        {// 3
            double nx = ToNumber(e, px);
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
            double ny = ToNumber(e, py);
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
            if(isnan(nx) || isnan(ny))
            {
                return Undefined::Instance();
            }
            if(nx == ny)
            {// this includes +0 vs -0
                return Bool::False();
            }
            if(isinf(nx) && !signbit(nx))
            {// nx = +inf
                return Bool::False();
            }
            if(isinf(ny) && !signbit(ny))
            {// ny = +inf
                return Bool::True();
            }
            if(isinf(ny) && signbit(ny))
            {// ny = -inf
                return Bool::False();
            }
            if(isinf(nx) && signbit(nx))
            {// nx = -inf
                return Bool::True();
            }
            return Bool::Wrap(nx < ny);
        }
        else
        {// 4
            std::string sx = ToString(e, px);
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
            std::string sy = ToString(e, py);
            if(!e->IsOk())
            {
                return Undefined::Instance();
            }
            return Bool::Wrap(sx < sy);
        }
    }

    // 11.9.3 The Abstract Equality Comparison Algorithm
    // x == y
    bool Equal(Error* e, JSValue* x, JSValue* y)
    {
        if(x->type() == y->type())
        {
            if(x->IsUndefined())
            {
                return true;
            }
            else if(x->IsNull())
            {
                return true;
            }
            else if(x->IsNumber())
            {
                Number* numx = static_cast<Number*>(x);
                Number* numy = static_cast<Number*>(y);
                if(numx->IsNaN() || numy->IsNaN())
                {
                    return false;
                }
                return numx->data() == numy->data();
            }
            else if(x->IsString())
            {
                String* sx = static_cast<String*>(x);
                String* sy = static_cast<String*>(y);
                return sx->data() == sy->data();
            }
            return x == y;
        }
        if(x->IsNull() && y->IsUndefined())
        {// 2
            return true;
        }
        else if(x->IsUndefined() && y->IsNull())
        {// 3
            return true;
        }
        else if(x->IsNumber() && y->IsString())
        {// 4
            double numy = ToNumber(e, y);
            if(!e->IsOk())
            {
                return false;
            }
            return Equal(e, x, new Number(numy));
        }
        else if(x->IsString() && y->IsNumber())
        {// 5
            double numx = ToNumber(e, x);
            if(!e->IsOk())
            {
                return false;
            }
            return Equal(e, new Number(numx), y);
        }
        else if(x->IsBool())
        {// 6
            double numx = ToNumber(e, x);
            if(!e->IsOk())
            {
                return false;
            }
            return Equal(e, new Number(numx), y);
        }
        else if(y->IsBool())
        {// 7
            double numy = ToNumber(e, x);
            if(!e->IsOk())
            {
                return false;
            }
            return Equal(e, x, new Number(numy));
        }
        else if(x->IsNumber() || x->IsString())
        {// 8
            JSValue* primy = ToPrimitive(e, y, "");
            if(!e->IsOk())
            {
                return false;
            }
            return Equal(e, x, primy);
        }
        else if(x->IsObject() && (y->IsNumber() || y->IsString()))
        {// 9
            JSValue* primx = ToPrimitive(e, y, "");
            if(!e->IsOk())
            {
                return false;
            }
            return Equal(e, primx, y);
        }
        return false;
    }

    // 11.9.6 The Strict Equality Comparison Algorithm
    // x === y
    bool StrictEqual(Error* e, JSValue* x, JSValue* y)
    {
        (void)e;
        if(x->type() != y->type())
        {
            return false;
        }
        switch(x->type())
        {
            case JSValue::JS_UNDEFINED:
                return true;
            case JSValue::JS_NULL:
                return true;
            case JSValue::JS_NUMBER:
            {
                Number* num_x = static_cast<Number*>(x);
                Number* num_y = static_cast<Number*>(y);
                if(num_x->IsNaN() || num_y->IsNaN())
                {
                    return false;
                }
                double dx = num_x->data();
                double dy = num_y->data();
                return dx == dy;
            }
            case JSValue::JS_STRING:
            {
                String* str_x = static_cast<String*>(x);
                String* str_y = static_cast<String*>(y);
                return str_x->data() == str_y->data();
            }
            case JSValue::JS_BOOL:
            {
                Bool* b_x = static_cast<Bool*>(x);
                Bool* b_y = static_cast<Bool*>(y);
                return b_x->data() == b_y->data();
            }
            default:
                return x == y;
        }
    }

}


