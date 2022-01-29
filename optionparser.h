/*
* Copyright 2017-2020 apfeltee (github.com/apfeltee)
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in the
* Software without restriction, including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* (it's the MIT license)
*/

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <stdexcept>
#include <cctype>

/* some features explicitly need minimum c++17 support */
#if ((__cplusplus != 201402L) && (__cplusplus < 201402L)) && (defined(_MSC_VER) && ((_MSC_VER != 1914) || (_MSC_VER < 1914)))
    #if !defined(_MSC_VER)
        #error "optionparser requires MINIMUM C++17!"
    #endif
#endif

/**
* OptionParser borrows the style of ruby's OptionParser, in that it
* uses callbacks. this makes it trivial to implement invocation of
* multiple values (think '-I' flag for gcc, for example), and,
* other than storing declarations in a vector, does not need to
* map out any values.
*
* the only drawback is, that value conversion (strings to integers, etc)
* has to be done manually.
*
* errors, if any, are thrown during parsing in gnopt::parse(), so
* a try/catch block would be necessary. again, this is virtually identical
* to how ruby's OptionParser does it.
*
* -----------
*
* sample usage:
*
*   OptionParser prs;
*   prs.on({"-o?", "--out=?"}, "set output file name", [&](const auto& val)
*   {
*       // 'val' is a OptionParser::Value, string value access via ::str()
*       myoutputfilename = val.str(); 
*   });
*   // without any values:
*   prs.on({"-v", "--verbose", "--setverbose"}, "toggle verbose output", [&]
*   {
*       setverbose = true;
*   });
*   prs.parse(argc, argv);
*   // or parse(argc, argv, <index>) if argc starts at a different index
*   // alternatively, parse() may also be used with an std::vector<string>
*   // unparsed values can be retrieved through prs.positional(), which is a vector of strings.
*
* -----------------------------------------
*
* things to do / implement:
*
*   - LLVM style options (single dash + fullstring, i.e., "-use-whatever", "use-whatever" being the key)
*
*   - "conditional" parsing, a la /bin/find. stuff like, say, "-if -then" ... which is to say
*     the *order* is important, not the actual flow. the order is kept in the same order they're
*     declared, since that's how arrays (more specifically, std::vector) work ...
*     just need to figure out a halfway sane api perhaps?
*
*   - help text generation is still kinda messy. it works, but could be better.
*
*   - parsing is a bit 
*/

/*
* note: bare-word `string` is not a typo, but typedef'd to std::basic_string!
* same with stringstream.
*/

template<typename CharT>
class BasicOptionParser
{
    public:
        struct Error: std::runtime_error
        {
            Error(const std::basic_string<CharT>& m): std::runtime_error(m)
            {
            }
        };

        struct InvalidOptionError: Error
        {
            using Error::Error;
        };

        struct ValueNeededError: Error
        {
            using Error::Error;
        };

        struct ValueConversionError: Error
        {
            using Error::Error;
        };

        struct IOError: Error
        {
            using Error::Error;
        };

        class Value;
        using string             = std::basic_string<CharT>;
        using stringstream       = std::basic_stringstream<CharT>;
        using StopIfCallback     = std::function<bool(BasicOptionParser&)>;
        using UnknownOptCallback = std::function<bool(const string&)>;
        using CallbackNoValue    = std::function<void()>;
        using CallbackWithValue  = std::function<void(const Value&)>;

        /*
        * this only used for native C++ - provides
        * deparsing string value into integers, etc.
        */
        class Value
        {
            private:
                string m_rawvalue;

            public: // static functions - doubles as helper function(s)
                template<typename OutType>
                static OutType lexical_convert(const string& str)
                {
                    OutType dest;
                    stringstream obuf;
                    obuf << str;
                    if(!(obuf >> dest))
                    {
                        // is possible to figure out *why* a conversion may have failed?
                        throw ValueConversionError("lexical_convert failed");
                    }
                    return dest;
                }

            public: // members
                Value(const string& raw): m_rawvalue(raw)
                {
                }

                template<typename OutType>
                OutType as() const
                {
                    return Value::lexical_convert<OutType>(m_rawvalue);
                }

                template<typename OutType>
                OutType convert() const
                {
                    return Value::lexical_convert<OutType>(m_rawvalue);
                }

                string str() const
                {
                    return m_rawvalue;
                }

                bool isEmpty() const
                {
                    return m_rawvalue.empty();
                }

                inline size_t size() const
                {
                    return m_rawvalue.size();
                }

                inline size_t length() const
                {
                    return size();
                }

                inline int operator[](int i) const
                {
                    return m_rawvalue[i];
                }
        };

        // some kind of bad variant type. or call it a hack.
        struct Callback
        {
            CallbackWithValue real_callback = nullptr;

            // default - no callback
            Callback()
            {
            }

            // a callback accepting a value
            Callback(CallbackWithValue cb)
            {
                real_callback = cb;
            }

            // a callback accepting no values
            Callback(CallbackNoValue cb)
            {
                /*
                * this hack is needed to for clr, since it requires
                * knowing what kind real_callback is - and there
                * doesn't seem to be a sane way to test it.
                */
                real_callback = [cb](const Value& v)
                {
                    (void)v;
                    return cb();
                };
            }

            void check()
            {
                if(real_callback == nullptr)
                {
                    throw std::runtime_error("real_callback is NULL");
                }
            }

            void invoke()
            {
                check();
                return real_callback(std::string());
            }

            void invoke(const string& s)
            {
                check();
                return real_callback(s);
            }
        };

        struct LongOption
        {
            string name;

            /*
            * this isn't really used yet - ideally, it would
            * be used to specify the type of parsing:
            * if true, it would parse GNU style options, like "--verbose", or "--prefix=foo".
            * if false, it would parse MS-DOS style options, like "/verbose" ("/v"), or "/out:whatever.txt".
            * this would also need a more thorough definition spec for short
            * options.
            *
            * tl;dr possible, but mixing these seems like a great way to needlessly
            * complicate everything
            */
            bool isgnu;
        };

        struct Declaration
        {
            // whether or not this decl has a short option (wip)
            bool has_short = false;

            // whether or not this decl has a long option (wip)
            bool has_long = false;

            // whether this decl needs a value. this is declared
            // through appending '?' to the short opt syntax, and '=?' to
            // the long opt syntax.
            bool needvalue = false;

            // whether this decl has a placeholder
            bool hasplaceholder = false;

            // the names of the short option (i.e., "a", "e", "i" ...)
            std::vector<CharT> shortnames;

            // the names of the long option (i.e., "out", "foo", "doathing")
            std::vector<LongOption> longnames;

            // the description (no special syntax!) (i.e., "set output file name")
            string description;

            // name of the placeholder (i.e., "--foo=<something>" = "something")
            string placeholder = "val";

            // the lambda/function to be called when this option is seen
            Callback callback;

            // a ref to OptionParser - used for alias
            BasicOptionParser* selfref;

            // return true if $c is recognized as short option
            inline bool is(CharT c) const
            {
                size_t i;
                for(i=0; i<shortnames.size(); i++)
                {
                    if(shortnames[i] == c)
                    {
                        return true;
                    }
                }
                return false;
            }

            // return true if $s is recognized as long option
            inline bool is(const string& s) const
            {
                size_t i;
                for(i=0; i<longnames.size(); i++)
                {
                    if(longnames[i].name == s)
                    {
                        return true;
                    }
                }
                return false;
            }

            inline string to_short_str() const
            {
                size_t i;
                stringstream buf;
                for(i=0; i<shortnames.size(); i++)
                {
                    buf << "-" << shortnames[i];
                    if(needvalue)
                    {
                        buf << "<" << placeholder << ">";
                    }
                    if((i + 1) != shortnames.size())
                    {
                        buf << " ";
                    }
                }
                return buf.str();
            }

            inline string to_long_str(size_t padsize=35) const
            {
                size_t i;
                size_t realpad;
                size_t tmplen;
                stringstream buf;
                stringstream tmp;
                tmp << to_short_str();
                tmp << " ";
                for(i=0; i<longnames.size(); i++)
                {
                    if(longnames[i].isgnu)
                    {
                        tmp << "--" << longnames[i].name;
                        if(needvalue)
                        {
                            tmp << "=<" << placeholder << ">";
                        }
                    }
                    else
                    {
                        tmp << "/" << longnames[i].name;
                        if(needvalue)
                        {
                            tmp << ":<" << placeholder << ">";
                        }
                    }
                    if((i + 1) < longnames.size())
                    {
                        tmp << ", ";
                    }
                }
                tmplen = tmp.tellp();
                realpad = ((tmplen <= padsize) ? padsize : (tmplen + 2));
                buf << "  " << tmp.str() << ":";
                while(true)
                {
                    buf << " ";
                    if(size_t(buf.tellp()) >= realpad)
                    {
                        break;
                    }
                }
                buf << description;
                return buf.str();
            }

            Declaration& alias(const std::vector<string>& opts);
        };

        /*
        * big TODO: read options from a file, i.e.,
        * if declaration is like on({"-v", "--verbose"}, ...), then
        * FileParser would look for token "verbose", and process it as
        * if passed on the command line.
        * like a configuration file, basically.
        * idea: add additional parameter to on() to explicitly specify
        * configuration file variable name (i.e., on({"-v", "--verbose"}, "use-verbose", "...", []{...}))
        */
        class FileParser
        {
            public:
            private:
                std::istream* m_stream;
                std::string m_filename;
                bool m_mustclose;

            private:
                std::istream* openFile(const std::string& path)
                {
                    std::fstream* strm;
                    strm = new std::fstream(path, std::ios::in | std::ios::binary);
                    return strm;
                }

                void check()
                {
                    std::stringstream ss;
                    if(!m_stream->good())
                    {
                        ss << "failed to open '" << m_filename << "' for reading";
                        throw IOError(ss.str());
                    }
                }

            public:
                // allow using an already existing stream
                // FileParser...(&alreadyexistingstream, "somefilename") ...
                FileParser(std::istream* strm, const std::string& filename, bool mustclose=false):
                    m_stream(strm), m_filename(filename), m_mustclose(mustclose)
                {
                    check();
                }

                FileParser(const std::string& path):
                    FileParser(openFile(path), true)
                {}

                ~FileParser()
                {
                    if(m_mustclose)
                    {
                        delete m_stream;
                    }
                }
        };

        // wrap around isalnum to permit '?', '!', '#', etc.
        static inline bool isalphanum(CharT c)
        {
            static const string other = "?!#";
            return (std::isalnum(int(c)) || (other.find_first_of(c) != string::npos));
        }

        // is it a valid long option?
        static inline bool isvalidlongopt(const string& str)
        {
            return ((str[0] == '-') && (str[1] == '-'));
        }

        // is it a valid DOS-style option?
        static inline bool isvaliddosopt(const string& str)
        {
            return ((str[0] == '/') && isalphanum(str[1]));
        }

    protected:
        // contains the argc/argv.
        std::vector<string> m_vargs;

        // contains unparsed, positional arguments. i.e, any non-options.
        std::vector<string> m_positional;

        // contains the option syntax declarations.
        std::vector<Declaration*> m_declarations;

        // stop_if callbacks
        std::vector<StopIfCallback> m_stopif_funcs;

        // buffer for the banner (the text printed prior to the help text)
        stringstream m_helpbannerbuf;

        // buffer for the tail (the text printed after the help text)
        stringstream m_helptailbuf;

        // true, if any DOS style options had been declared.
        // only meaningful in parse() - DOS options are usually ignored.
        bool m_dosoptsdeclared = false;

        bool m_stopparsing = false;

        // a handler for unknown/errornous options
        UnknownOptCallback m_on_unknownoptfn;

    protected:
        /*
        * todo: more meaningful exception classes
        * this function is also meant as a wrapper for situations
        * where a C++ compiler doesn't allow exceptions (for any reason).
        * needs to be ifdef'd, for those cases.
        */
        template<typename ExClass, typename... Args>
        void throwError(Args&&... args)
        {
            string msg;
            stringstream buf;
            ((buf << args), ...);
            msg = buf.str();
            throw ExClass(msg);
        }

        /*
        * wraparound for invoke_on_unknown.
        */
        inline bool invoke_on_unknown_prox(const string& optstr)
        {
            if(m_on_unknownoptfn == nullptr)
            {
                return true;
            }
            return m_on_unknownoptfn(optstr);
        }

        /*
        * called, when an unknown long option (i.e., '--foo') is encountered.
        */
        inline bool invoke_on_unknown(const string& str)
        {
            string ostr;
            ostr.append("--");
            ostr.append(str);
            return invoke_on_unknown_prox(ostr);
        }

        /*
        * called, when an unknown short option (i.e., '-c') is encountered.
        */
        inline bool invoke_on_unknown(CharT opt)
        {
            string optstr;
            optstr.push_back('-');
            optstr.push_back(CharT(opt));
            return invoke_on_unknown_prox(optstr);
        }

        /*
        * wraps exception to either be thrown, or forwarded to invoke_on_unknown.
        * useful for when exception are unavailable (i think? never encountered such a scenario).
        */
        template<typename ExceptionT, typename ValType, typename... Args>
        inline void invoke_or_throw(const ValType& val, size_t& iref, size_t howmuch, Args&&... args)
        {
            size_t tmp;
            if(invoke_on_unknown(val))
            {
                throwError<ExceptionT>(args...);
            }
            tmp = (iref + howmuch);
            if((tmp + 1) != m_vargs.size())
            {
                iref = tmp;
            }
        }

        /*
        * optpat = input option decl string, i.e., "--foo=<bar>"
        * dest = where placeholder string is stored
        * subend = where begin of placeholder is stored
        * islong = (unused... for now)
        */
        bool hasplaceholder(const std::string& optpat, std::string& dest, size_t& subend, bool islong)
        {
            size_t ibegin;
            char end;
            (void)islong;
            end = *(optpat.end() - 1);
            if(end == '>')
            {
                ibegin = optpat.find_first_of('<');
                if(ibegin < optpat.size())
                {
                    subend = ibegin - 3;
                    // substr behaves weirdly. why can't C++ just be normal?
                    dest = std::string(optpat.begin() + (ibegin+1), optpat.end() - 1);
                    return true;
                }
            }
            return false;
        }

        /*
        * parse option declarations $strs into a Declaration instance.
        *
        * short option (getopt style):
        *   # -v
        *   # -o?
        *   "-" <char> ( "?" | ( "<" <string> ">" )
        *
        * long option (GNU style):
        *   # --verbose
        *   # --outputfile=?
        *   "--" <string> ( "=?" | ( "=<" <string> ">" ) )
        *
        * long option (DOS style):
        *   # /?
        *   # /v
        *   # /verbose
        *   # /out:?
        *   "/" <string> ( ":?" | ( ":<" <string> ">" )
        *
        * 'alnum' is alphanumeric, i.e., alphabet (uppercase & lowercase) + digits.
        */
        inline Declaration& addDeclaration(const std::vector<string>& strs, const string& desc, Callback fn)
        {
            size_t i;
            size_t subend;
            bool hph;
            bool isgnu;
            bool hadlongopts;
            bool hadshortopts;
            bool longwantvalue;
            bool shortwantvalue;
            Declaration* decl;
            string longstr;
            string shortstr;
            string longname;
            string placeholder;
            CharT shortname;
            CharT shortbegin;
            CharT shortend;
            CharT longbegin1;
            CharT longbegin2;
            CharT longend;
            CharT longeq;
            // create new decl. note that a ref to the pointer is returned, since
            // the pointer is stored anyway.
            decl = new Declaration;
            hph = false;
            hadlongopts = false;
            hadshortopts = false;
            longwantvalue = false;
            shortwantvalue = false;
            decl->selfref = this;
            decl->description = desc;
            decl->callback = fn;
			(void)shortbegin;
            if(strs.size() == 0)
            {
                // return, but don't actually do anything ....
                // this isn't technically an error, but it will be completely ignored
                return *decl;
            }
            for(i=0; i<strs.size(); i++)
            {
                /*
                * grammar (pseudo):
                *
                *   ("--" | "/") <string> (("=" | ":") ("?" | ("<" <string> ">")))
                *
                * if DOS syntax is used (i.e., "/out"), which would need a value, it
                * needs to be declared as "/out:?", or "/out:<thing>", but NOT as "/out=?".
                * likewise, if GNU syntax is used, the decl needs to be "--out=?", but
                * NOT "--out:?".
                * the DOS variant must be immediately followed by an alpanumeric char.
                * (don't use DOS decls, unless you mean to port backwards. it's archaic af)
                */
                if(isvalidlongopt(strs[i]) || isvaliddosopt(strs[i]))
                {
                    /*
                    * longbegin1 is the first char in longstr
                    * longbegin2 is the second char in longstr
                    * longend is the last char in longstr
                    * longeq is the char before the last char in longstr
                    */
                    hadlongopts = true;
                    longstr = strs[i];
                    longbegin1 = longstr[0];
                    longbegin2 = longstr[1];
                    longend = (*(longstr.end() - 1));
                    longeq = (*(longstr.end() - 2));
                    isgnu = ((longbegin1 == '-') && (longbegin2 == '-'));
                    if(isgnu)
                    {
                        /* "--foo=?" = $length - len("=?") */
                        subend = longstr.size() - 4;
                        /* allow on({"--foo=<name>"})*/
                        longwantvalue = (
                            ((longeq == '=') && (longend == '?')) ||
                            (hph = hasplaceholder(longstr, placeholder, subend, true))
                        );
                        if(longwantvalue)
                        {
                            longname = longstr.substr(2).substr(0, subend);
                            if(hph)
                            {
                                decl->hasplaceholder = true;
                                decl->placeholder = placeholder;
                            }
                        }
                        else
                        {
                            longname = longstr.substr(2);
                        }
                    }
                    // ms-style options can be '/foo:bar', but also '-foo:bar'
                    else if((longbegin1 == '/') || (longbegin1 == '-'))
                    {
                        // "/foo:?" = $length - len(":?")
                        subend = longstr.size() - 3;
                        longwantvalue = (
                            ((longeq == ':') && (longend == '?')) ||
                            (hph=hasplaceholder(longstr, placeholder, subend, false))
                        );
                        if(longwantvalue)
                        {
                            longname = longstr.substr(1).substr(0, subend);
                            if(hph)
                            {
                                decl->hasplaceholder = true;
                                decl->placeholder = placeholder;
                            }
                        }
                        else
                        {
                            longname = longstr.substr(1);
                        }
                        m_dosoptsdeclared = true;
                    }
                    else
                    {
                        delete decl;
                        throwError<Error>("impossible situation: failed to parse '", longstr, "'");
                    }
                    decl->longnames.push_back(LongOption{longname, isgnu});
                }
                /*
                * grammar (pseudo): "-" <char:alnum> ("?")
                * also allows declaring numeric flags, i.e., "-0" (like grep, sort of)
                * todo: allow numeric flags as a range - perhaps a wrapper function that
                * auto-populates an array of options with the range of numbers?
                */
                else if((strs[i][0] == '-') && isalphanum(strs[i][1]))
                {
                    /*
                    * shortbegin is the first char in shortstr
                    * shortname is the second char in shortstr
                    * shortend is the last char in shortstr
                    */
                    hadshortopts = true;
                    shortstr = strs[i];
                    shortbegin = shortstr[0];
                    shortname = shortstr[1];
                    shortend = *(shortstr.end() - 1);
                    // permits declaring '-?'
                    shortwantvalue = ((shortend == '?') && (shortend != shortname));
                    hph = hasplaceholder(shortstr, placeholder, subend, false);
                    decl->shortnames.push_back(shortname);
                    if(hph)
                    {
                        shortwantvalue = true;
                        decl->hasplaceholder = true;
                        decl->placeholder = placeholder;
                    }
                }
                else
                {
                    delete decl;
                    throwError<Error>("unparseable option syntax '", strs[i],"'");
                }
            }
            /*
            * ensure parsing state sanity: if one option requires a value, then
            * every other option must too.
            * TODO: optional mandatory value (silly sentence), like some
            * clang options?
            * perhaps what ruby's optparse does, i.e., "--foo[=<val>]", or "-f[<val>]", which
            * then ONLY recognizes <val> if specified via "--foo=bar" or "-fbar" respectively, but
            * NOT "--foo bar", or "-f bar", respectively. this would be just a simple flag,
            * 'optionalvalue', and the Value class would need to reflect that as well.
            */
            if(longwantvalue == true)
            {
                if((shortwantvalue == false) && (hadshortopts == true))
                {
                    delete decl;
                    throwError<Error>("long option ended in '=?', but short option did not");
                }
            }
            if(shortwantvalue == true)
            {
                if((longwantvalue == false) && (hadlongopts == true))
                {
                    delete decl;
                    throwError<Error>("short option ended in '?', but long option did not");
                }
            }
            decl->needvalue = (longwantvalue && shortwantvalue);
            m_declarations.push_back(decl);
            return *decl;
        }

        inline bool find_decl_long(const string& name, Declaration& decldest, size_t& idxdest)
        {
            size_t i;
            for(i=0; i<m_declarations.size(); i++)
            {
                if(m_declarations[i]->is(name))
                {
                    idxdest = i;
                    decldest = *(m_declarations[i]);
                    return true;
                }
            }
            return false;
        }

        inline bool find_decl_short(CharT name, Declaration& decldest, size_t& idxdest)
        {
            size_t i;
            for(i=0; i<m_declarations.size(); i++)
            {
                if(m_declarations[i]->is(name))
                {
                    idxdest = i;
                    decldest = *(m_declarations[i]);
                    return true;
                }
            }
            return false;
        }

        /*
        * parse a short option with more than one character, OR combined options.
        * sometimes refered to as GNU-style options.
        */
        inline void parse_multishort(const string& str, size_t& iref)
        {
            size_t i;
            size_t idx;
            string ovalue;
            Declaration decl;
            idx = 0;
            for(i=0; i<str.size(); i++)
            {
                /*
                * $idx is the index of the option in question of the multishort string.
                * i.e., if $str is "-ovd", and '-o' expects a value, then $idx-1 is the
                * index of 'o' in $str
                */
                if(find_decl_short(str[i], decl, idx))
                {
                    if(decl.needvalue && (i == 0))
                    {
                        if(str.size() > 1)
                        {
                            ovalue = str.substr(1);
                            decl.callback.invoke(ovalue);
                            return;
                        }
                        else
                        {
                            throwError<ValueNeededError>("option '-", str[idx-1], "' expected a value");
                        }
                    }
                    else
                    {
                        /*
                        * a short option combined with other opts was passed, which
                        * also expected a value. afaik, this would result in an error
                        * in GNU getopt as well
                        */
                        if(decl.needvalue)
                        {
                            throwError<ValueNeededError>("unexpected option '-", str[idx-1], "' requiring a value");
                        }
                        else
                        {
                            decl.callback.invoke();
                        }
                    }
                }
                else
                {
                    // invoke_on_unknown: multishort
                    invoke_or_throw<InvalidOptionError>(str[i], iref, 0,
                        "unknown short option '-", str[i], "'");
                    /*
                    * if we don't return here, then it will just return back to this block,
                    * unless, by chance, the string(s) happen to contain an option
                    * we can parse, and EVEN SO it would be still just a game of chance.
                    * best to go the safe way, and give up on it entirely.
                    * pessimistic, maybe, but the least likely to introduce bugs.
                    */
                    return;
                }
            }
        }

        inline void parse_simpleshort(CharT str, size_t& iref)
        {
            size_t idx;
            Declaration decl;
            string value;
            if(find_decl_short(str, decl, idx))
            {
                if(decl.needvalue)
                {
                    /*
                    * decl wants a value, so grab value from the next argument, if
                    * the next arg isn't an option, and increase index
                    */
                    if((iref+1) < m_vargs.size())
                    {
                        /*
                        * make sure the next argument isn't some sort of option;
                        * even if it is just a double dash ("--"). if it starts with
                        * a dash, it's no good.
                        * otherwise, something like "-o -foo" would yield "-foo"
                        * as value!
                        */
                        if(m_vargs[iref+1][0] != '-')
                        {
                            value = m_vargs[iref + 1];
                            iref++;
                            decl.callback.invoke(value);
                            return;
                        }
                    }
                    throwError<ValueNeededError>("option '-", str, "' expected a value");
                }
                else
                {
                    decl.callback.invoke();
                }
            }
            else
            {
                // invoke_on_unknown: simpleshort
                invoke_or_throw<InvalidOptionError>(str, iref, 0, "unknown option '-", str, "'");
            }
        }

        /*
        * parse an argument string that matches the pattern of
        * a long option, extract its values (if any), and invoke callbacks.
        * AFAIK long options can't be combined in GNU getopt, so neither does this function.
        */
        void parse_longoption(const string& argstring, size_t& iref)
        {
            size_t idx;
			size_t eqpos;
            string name;
            string nodash;
            string value;
            Declaration decl;
            nodash = argstring.substr(2);
            eqpos = argstring.find_first_of('=');
            if(eqpos == string::npos)
            {
                name = nodash;
            }
            else
            {
                /* get name by cutting nodash until eqpos */
                name = nodash.substr(0, eqpos - 2);
            }
            if(find_decl_long(name, decl, idx))
            {
                if(decl.needvalue)
                {
                    if(eqpos == string::npos)
                    {
                        throwError<ValueNeededError>("option '", name, "' expected a value");
                    }
                    else
                    {
                        /* get value by cutting nodash after eqpos */
                        value = nodash.substr(eqpos-1);
                        decl.callback.invoke(value);
                    }
                }
                else
                {
                    decl.callback.invoke();
                }
            }
            else
            {
                // invoke_on_unknown: longoption
                invoke_or_throw<InvalidOptionError>(name, iref, 0, "unknown option '", name, "'");
            }
        }

        bool realparse()
        {
            size_t i;
            string nodash;
            m_stopparsing = false;
            for(i=0; i<m_vargs.size(); i++)
            {

                for(auto iter=m_stopif_funcs.begin(); iter!=m_stopif_funcs.end(); iter++)
                {
                    if((*iter)(*this))
                    {
                        m_stopparsing = true;
                        break;
                    }
                }
                /*
                * GNU behavior feature: double-dash means to stop parsing arguments, but
                * only if it wasn't signalled already by stop_if
                */
                if((m_vargs[i] == "--") && (m_stopparsing == false))
                {
                    m_stopparsing = true;
                    continue;
                }
                if(m_stopparsing)
                {
                    m_positional.push_back(m_vargs[i]);
                }
                else
                {
                    if(m_vargs[i][0] == '-')
                    {
                        /* arg starts with "--", so it's a long option. */
                        if(m_vargs[i][1] == '-')
                        {
                            parse_longoption(m_vargs[i], i);
                        }
                        else
                        {
                            nodash = m_vargs[i].substr(1);
                            /*
                            * arg starts with "-", but has more than one character.
                            * in this case, it could be combined options without arguments
                            * (something like '-v' for verbose, '-d' for debug, etc),
                            * but it could also be an option with argument, i.e., '-ofoo',
                            * where '-o' is the option, and 'foo' is the value.
                            */
                            if(nodash.size() > 1)
                            {
                                parse_multishort(nodash, i);
                            }
                            else
                            {
                                /*
                                * process simple short option (e.g., "-ofoo", but also "-o" "foo")
                                * by passing current index as reference.
                                * that is, parse_simpleshort may increase index if option
                                * requires a value, otherwise i remains as-is.
                                */
                                parse_simpleshort(nodash[0], i);
                            }
                        }
                    }
                    /*
                    * todo: DOS style command parsing:
                    * only process if any DOS style options were actually declared, since
                    * this is going to cause all sorts of mixups with positional arguments.
                    * additionally, process invalid and/or unknown DOS options as positional
                    * arguments, since this is more or less what windows seems to do
                    */
                    /*
                    if(m_dosoptsdeclared)
                    {
                        ...
                    }
                    */
                    else
                    {
                        m_positional.push_back(m_vargs[i]);
                    }
                }
            }
            return true;
        }

        /*
        * todo: cuddle short options that take no arguments
        */
        template<typename StreamT>
        StreamT& help_declarations_short(StreamT& buf) const
        {
            size_t i;
            for(i=0; i<m_declarations.size(); i++)
            {
                buf << "[" << m_declarations[i]->to_short_str() << "]";
                if((i + 1) != m_declarations.size())
                {
                    buf << " ";
                }
            }
            return buf;
        }

        template<typename StreamT>
        StreamT& help_declarations_long(StreamT& buf) const
        {
            size_t i;
            for(i=0; i<m_declarations.size(); i++)
            {
                buf << m_declarations[i]->to_long_str() << std::endl;
            }
            return buf;
        }

        void init(bool declhelp)
        {
            if(declhelp)
            {
                this->on({"-h", "--help"}, "show this help", [&]
                {
                    this->help(std::cout);
                    std::exit(0);
                });
            }
        }

    #if defined(__cplusplus_cli)
    public:
        void cliboilerplate_pushvarg(const string& v)
        {
            m_vargs.push_back(v);
        }
        /*
        * realparse() is intended to be protected - but C++CLR won't let me touch its privates.
        * bummer
        */
        bool cliboilerplate_realparse()
        {
            return realparse();
        }
    #endif

    public:
        /**
        * initializes OptionParser().
        *
        * @param declhelp  if true, will define a default response to "-h", "-?" and "--help",
        *                  printing help to standard output, and calls exit(0).
        */
        BasicOptionParser(bool declhelp=true)
        {
            init(declhelp);
        }

        virtual ~BasicOptionParser()
        {
            for(auto decl: m_declarations)
            {
                delete decl;
            }
        }

        void stopParsing()
        {
            m_stopparsing = true;
        }

        /**
        * add an option declaration.
        *
        * @param strs   a vector of option syntaxes. for example:
        *               {"-o?", "--out=?"}
        *               declares a short option "-o" taking a value, such as "-ofoo", or "-o foo",
        *               and declares "--out", taking a value, like "--out=foo" (but *NOT* "--out foo")
        *
        * @param desc   the description for this option. it's used to generate the help()
        *               text.
        *
        * @param fn     the callback used to call when this option is seen.
        *               it can either be an empty lambda (i.e., [&]{ ... code ... }), or
        *               taking a string (i.e., [&](string s){ ... code ... }).
        *               it is only called when parsing is valid, so if "--out" was declared
        *               to take a value, but is missing a value, it is *not* called.
        *               it will also be called for *every* instance found, so
        *               for example, an option declared "-I?"  can be called multiple times
        *               to, for example, build a vector of values.
        */
        /*
        * param fn used to be a template, but code generators keep getting it wrong. so there.
        */
        Declaration& on(const std::vector<string>& strs, const string& desc, CallbackWithValue fn)
        {
            return addDeclaration(strs, desc, Callback(fn));
        }

        Declaration& on(const std::vector<string>& strs, const string& desc, CallbackNoValue fn)
        {
            return addDeclaration(strs, desc, Callback(fn));
        }

        /***
        * declare a callback that is called whenever an unknown/undeclared option flag
        * is encountered.
        * the callback must match `bool(const string&)`, and must return
        * either true, or false.
        * the argument passed to the callback is the option string as it was seen
        * by the parser. note: with the exception of the amount of hyphens, you should NOT make
        * any assumptions as to what the flag might mean!
        *
        * return true:
        *   if the callback returns true, then the exception InvalidOptionError
        *   is raised, and parsing will be halted entirely.
        *   this is the default.
        *
        * return false:
        *   if it returns false, then optionparser will continue with the
        *   next option flags (if any), without raising any exceptions.
        */
        void onUnknownOption(UnknownOptCallback fn)
        {
            m_on_unknownoptfn = fn;
        }

        /**
        * reference to the help() banner stream.
        * the banner is the text shown before the help text.
        */
        inline stringstream& banner()
        {
            return m_helpbannerbuf;
        }

        /**
        * reference to the help() tail stream.
        * the tail is the text shown after the help text.
        */
        inline stringstream& tail()
        {
            return m_helptailbuf;
        }

        /**
        * returns a nice help text, based on the option declarations, and
        * their descriptions, as well as the banner stream, and the tail stream.
        *
        * @param buf   a ostream-compatible stream (that defines operator<<()) to
        *              write the text to.
        */
        template<typename StreamT>
        StreamT& help(StreamT& buf) const
        {
            buf << m_helpbannerbuf.str() << std::endl;
            buf << "usage: ";
            help_declarations_short(buf);
            buf << " <args ...>" << std::endl << std::endl;
            buf << "available options:" << std::endl;
            help_declarations_long(buf);
            buf << m_helptailbuf.str() << std::endl;
            return buf;
        }

        /**
        * like help(StreamT&), but writes to a stringstream, and returns a string.
        */
        inline string help() const
        {
            stringstream buf;
            return help(buf).str();
        }

        /**
        * returns the positional (non-parsed) values, that are leftover from parsing.
        */
        inline std::vector<string> positional() const
        {
            return m_positional;
        }

        /**
        * returns argument idx of the positional values.
        */
        inline std::string positional(size_t idx) const
        {
            return m_positional[idx];
        }

        /**
        * returns the amount of positional values.
        */
        inline size_t size() const
        {
            return m_positional.size();
        }

        /**
        * like size(), but for C++-isms.
        */
        inline size_t length() const
        {
            return this->size();
        }

        /**
        * add a function that is called prior to each parsing loop, determining
        * whether or not to stop parsing.
        * for an example, look at the implementation of stopIfSawPositional().
        */
        inline void stopIf(StopIfCallback cb)
        {
            m_stopif_funcs.push_back(cb);
        }

        /**
        * adds a StopIfCallback causing the parser to stop parsing, and treat every
        * argument as a positional value IF a non-option (positional value) was seen.
        *
        * i.e., if argv is something like {"-o", "foo", "-I/opt/stuff", "things.c", "-I/something/else"}
        * then this would cause the parser to stop parsing after having seen "things.c", and the
        * positional values would yield {"things.c", "-I/something/else"}, which in turn
        * would NOT call the callback for the definition of "-I?", since it was not parsed.
        * useful for programs that parse arguments similar to strace, i.e.:
        *
        *   mysillystrace -log.txt --what=io mysillyprogram --someoption=foo bar quux
        *
        * which, when used with stopIfSawPositional(), would mean the positional values
        * would be {"mysillyprogram", "--someoption=foo", "bar", "quux"}.
        */
        inline void stopIfSawPositional()
        {
            this->stopIf([](BasicOptionParser& p)
            {
                return (p.size() > 0);
            });
        }

        /**
        * populate m_vargs, and call the parser with argc/argv as it were passed
        * to main().
        *
        * @param argc    the argument vector count.
        * @param argv    the argument vector values.
        * @param begin   the index at which to begin collecting values.
        *                there is usually no reason to specify this explicitly,
        *                unless your operating system populates argc/argv in a
        *                odd non-standard way.
        * @returns true if parsing succeeded. only really usable when exceptions
        *          are disabled.
        */
        bool parse(int argc, char** argv, int begin=1)
        {
            int i;
            m_vargs.reserve(argc + 1);
            for(i=begin; i<argc; i++)
            {
                m_vargs.push_back(argv[i]);
            }
            return realparse();
        }

        /**
        * like parse(int, char**, int), but with a std::vector.
        * unlike parse(int, char**, int) however, it will assume that the
        * index starts at 0.
        */
        bool parse(const std::vector<string>& args)
        {
            m_vargs = args;
            return realparse();
        }
};

/* in c++clr mode, OptionParser is defined in wrap.cpp */
#if !defined(_OPTIONPARSER_CLRMODE)
using OptionParser = BasicOptionParser<char>;
#endif

// that's all, folks!
