
#include "es.h"

namespace es
{
    std::string NumberToString(double m)
    {
        int k;
        int n;
        int frac_digit;
        size_t i;
        double s;
        double tmp;
        std::string res;
        std::string sign;
        // TODO(zhuzilin) Figure out how to solve the large number error.
        if(m == 0)
        {
            return String::Zero()->data();
        }
        if(m < 0)
        {
            m = -m;
            sign = "-";
        }
        // the fraction digits, e.g. 1.23's frac_digit = 2, 4200's = -2
        frac_digit = 0;
        // the total digits, e.g. 1.23's k = 3, 4200's k = 2
        k = 0;
        // n - k = -frac_digit
        n = 0;
        while(modf(m, &tmp) != 0)
        {
            frac_digit++;
            m *= 10;
        }
        while(fmod(m, 10) < 1e-6)
        {
            frac_digit--;
            m /= 10;
        }
        s = m;
        while(m > 0.5)
        {
            k++;
            m /= 10;
            modf(m, &tmp);
            m = tmp;
        }
        n = k - frac_digit;
        if(k <= n && n <= 21)
        {
            while(s > 0.5)
            {
                res += u'0' + int(fmod(s, 10));
                s /= 10;
                modf(s, &tmp);
                s = tmp;
            }
            reverse(res.begin(), res.end());
            res += std::string(n - k, u'0');
            return sign + res;
        }
        if(0 < n && n <= 21)
        {
            for(i = 0; i < size_t(k); i++)
            {
                res += u'0' + int(fmod(s, 10));
                if(int(int(i) + 1) == int(int(k) - int(n)))
                {
                    res += u'.';
                }
                s /= 10;
                modf(s, &tmp);
                s = tmp;
            }
            reverse(res.begin(), res.end());
            return sign + res;
        }
        if(-6 < n && n <= 0)
        {
            for(i = 0; i < size_t(k); i++)
            {
                res += u'0' + int(fmod(s, 10));
                s /= 10;
                modf(s, &tmp);
                s = tmp;
            }
            reverse(res.begin(), res.end());
            res = "0." + std::string(-n, u'0') + res;
            return sign + res;
        }
        if(k == 1)
        {
            res += u'0' + int(s);
            res += "e";
            if(n - 1 > 0)
            {
                res += "+" + NumberToString(n - 1);
            }
            else
            {
                res += "-" + NumberToString(1 - n);
            }
            return sign + res;
        }
        for(i = 0; i < size_t(k); i++)
        {
            res += u'0' + int(fmod(s, 10));
            if(int(int(i) + 1) == int(int(k) - 1))
            {
                res += u'.';
            }
            s /= 10;
            modf(s, &tmp);
            s = tmp;
        }
        res += "e";
        if(n - 1 > 0)
        {
            res += "+" + NumberToString(n - 1);
        }
        else
        {
            res += "-" + NumberToString(1 - n);
        }
        return sign + res;
    }

    std::string NumberToString(Number* num)
    {
        if(num->IsNaN())
        {
            return String::NaN()->data();
        }
        if(num->IsInfinity())
        {
            return String::Infinity()->data();
        }
        return NumberToString(num->data());
    }

    double ToInt32(Error* e, JSValue* input)
    {
        double num = ToNumber(e, input);
        if(!e->IsOk())
        {
            return 0;
        }
        if(isnan(num) || isinf(num) || num == 0)
        {
            return 0.0;
        }
        double pos_int = num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
        double int32_bit = fmod(pos_int, pow(2, 32));
        if(int32_bit < 0)
        {
            int32_bit += pow(2, 32);
        }

        if(int32_bit > pow(2, 31))
        {
            return int32_bit - pow(2, 32);
        }
        else
        {
            return int32_bit;
        }
    }

    double ToUint(Error* e, JSValue* input, char bits)
    {
        double num = ToNumber(e, input);
        if(!e->IsOk())
        {
            return 0.0;
        }
        if(isnan(num) || isinf(num) || num == 0)
        {
            return 0.0;
        }
        double pos_int = num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
        double int_bit = fmod(pos_int, pow(2, bits));
        if(int_bit < 0)
        {
            int_bit += pow(2, bits);
        }
        return int_bit;
    }

    double ToUint32(Error* e, JSValue* input)
    {
        return ToUint(e, input, 32);
    }

    double ToUint16(Error* e, JSValue* input)
    {
        return ToUint(e, input, 16);
    }

    double StringToNumber(String* str)
    {
        return StringToNumber(str->data());
    }

    double ToNumber(Error* e, JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
                return nan("");
            case JSValue::JS_NULL:
                return 0.0;
            case JSValue::JS_BOOL:
                return static_cast<Bool*>(input)->data() ? 1.0 : 0.0;
            case JSValue::JS_NUMBER:
                return static_cast<Number*>(input)->data();
            case JSValue::JS_STRING:
                return StringToNumber(static_cast<String*>(input));
            case JSValue::JS_OBJECT:
            {
                JSValue* prim_value = ToPrimitive(e, input, "Number");
                if(!e->IsOk())
                {
                    return 0.0;
                }
                return ToNumber(e, prim_value);
            }
            default:
                assert(false);
        }
    }

    double ToInteger(Error* e, JSValue* input)
    {
        double num = ToNumber(e, input);
        if(!e->IsOk())
        {
            return 0.0;
        }
        if(isnan(num))
        {
            return 0.0;
        }
        if(isinf(num) || num == 0)
        {
            return num;
        }
        return num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
    }

    double StringToNumber(const std::string& source)
    {
        size_t start = 0;
        size_t end = source.size();
        bool positive = true;
        double val = 0;
        while(start < end)
        {
            char c = source[start];
            if(!character::IsWhiteSpace(c) && !character::IsLineTerminator(c))
            {
                break;
            }
            start++;
        }
        while(start < end)
        {
            char c = source[end - 1];
            if(!character::IsWhiteSpace(c) && !character::IsLineTerminator(c))
            {
                break;
            }
            end--;
        }
        if(start == end)
        {
            goto error;
        }
        else if(source[start] == u'-')
        {
            positive = false;
            start++;
        }
        else if(source[start] == u'+')
        {
            start++;
        }
        else if(end - start > 2 && source[start] == u'0' && (source[start + 1] == u'x' || source[start + 1] == u'X'))
        {
            // 0xABCD...
            start += 2;
            while(start < end)
            {
                char c = source[start];
                if(!character::IsHexDigit(c))
                {
                    goto error;
                }
                val = val * 16 + character::Digit(c);
                start++;
            }
            return val;
        }

        if(start == end)
        {
            goto error;
        }

        if(source.substr(start, end - start) == "Infinity")
        {
            return positive ? Number::PositiveInfinity()->data() : Number::NegativeInfinity()->data();
        }

        while(start < end)
        {
            char c = source[start];
            if(!character::IsDecimalDigit(c))
            {
                break;
            }
            val = val * 10 + character::Digit(c);
            start++;
        }
        if(start == end)
        {
            return positive ? val : -val;
        }

        if(source[start] == u'.')
        {
            start++;
            double frac = 0.1;
            while(start < end)
            {
                char c = source[start];
                if(!character::IsDecimalDigit(c))
                {
                    break;
                }
                val += frac * character::Digit(c);
                frac /= 10;
                start++;
            }
            if(start == end)
            {
                return positive ? val : -val;
            }
        }

        if(source[start] == u'e' || source[start] == u'E')
        {
            start++;
            double exp = 0;
            bool exp_positive = true;
            if(start == end)
            {
                goto error;
            }
            if(source[start] == u'-')
            {
                exp_positive = false;
                start++;
            }
            else if(source[start] == u'-')
            {
                start++;
            }
            if(start == end)
            {
                goto error;
            }

            while(start < end)
            {
                char c = source[start];
                if(!character::IsDecimalDigit(c))
                {
                    break;
                }
                exp = exp * 10 + character::Digit(c);
                start++;
            }
            if(start == end)
            {
                val = val * pow(10, exp);
                return positive ? val : -val;
            }
        }
    error:
        return nan("");
    }

    bool ToBoolean(JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
            case JSValue::JS_NULL:
                return false;
            case JSValue::JS_BOOL:
                return static_cast<Bool*>(input)->data();
            case JSValue::JS_NUMBER:
            {
                Number* num = static_cast<Number*>(input);
                return !(num->data() == 0.0 || num->data() == -0.0 || num->IsNaN());
            }
            case JSValue::JS_STRING:
            {
                String* str = static_cast<String*>(input);
                return str->data().empty();
            }
            case JSValue::JS_OBJECT:
                return true;
            default:
                assert(false);
        }
    }

    JSValue* ToPrimitive(Error* e, JSValue* input, const std::string& preferred_type)
    {
        assert(input->IsLanguageType());
        if(input->IsPrimitive())
        {
            return input;
        }
        JSObject* obj = static_cast<JSObject*>(input);
        return obj->DefaultValue(e, preferred_type);
    }

    std::string ToString(Error* e, JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
                return String::Undefined()->data();
            case JSValue::JS_NULL:
                return String::Null()->data();
            case JSValue::JS_BOOL:
                return static_cast<Bool*>(input)->data() ? "true" : "false";
            case JSValue::JS_NUMBER:
                return NumberToString(static_cast<Number*>(input));
            case JSValue::JS_STRING:
                return static_cast<String*>(input)->data();
            case JSValue::JS_OBJECT:
            {
                JSValue* prim_value = ToPrimitive(e, input, "String");
                if(!e->IsOk())
                {
                    return "";
                }
                return ToString(e, prim_value);
            }
            default:
                assert(false);
        }
    }

}


