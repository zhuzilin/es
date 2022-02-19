import os
import argparse

parser = argparse.ArgumentParser(description="")
parser.add_argument("es", type=str)
parser.add_argument("test262_path", type=str)

args = parser.parse_args()

current_path = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(current_path, "harness.js")) as f:
    harness = f.read()

not_fixed_tests = [
  # ch07
  "../test262/test/suite/ch07/7.8/7.8.1/S7.8.1_A1_T2.js",  # RegExp
  # ch08
  "../test262/test/suite/ch08/8.5/S8.5_A13_T2.js",  # infinity
  "../test262/test/suite/ch08/8.5/8.5.1.js",        # infinity and small number
  # ch09
  "../test262/test/suite/ch09/9.2/S9.2_A6_T2.js",          # Date
  "../test262/test/suite/ch09/9.2/S9.2_A6_T1.js",          # Date
  "../test262/test/suite/ch09/9.3/S9.3_A4.1_T2.js",        # infinity
  "../test262/test/suite/ch09/9.3/S9.3_A4.1_T1.js",        # infinity
  "../test262/test/suite/ch09/9.3/9.3.1/S9.3.1_A3_T1.js",  # v8 cannot pass as well...
  "../test262/test/suite/ch09/9.3/9.3.1/S9.3.1_A32.js",    # precision
  "../test262/test/suite/ch09/9.3/9.3.1/S9.3.1_A3_T2.js",  # v8 cannot pass as well...
  "../test262/test/suite/ch09/9.3/9.3.1/S9.3.1_A2.js",     # v8 cannot pass as well...
  "../test262/test/suite/ch09/9.4/S9.4_A3_T2.js",          # Date
  "../test262/test/suite/ch09/9.4/S9.4_A3_T1.js",          # Date
  "../test262/test/suite/ch09/9.8/9.8.1/S9.8.1_A10.js",    # double to string
  "../test262/test/suite/ch09/9.8/9.8.1/S9.8.1_A8.js",     # double to string
  "../test262/test/suite/ch09/9.8/9.8.1/S9.8.1_A9_T2.js",  # double to string
  # ch10
  "../test262/test/suite/ch10/10.4/10.4.2/10.4.2-1-5.js",   # indirect eval
  "../test262/test/suite/ch10/10.4/10.4.2/10.4.2-1-2.js",   # indirect eval
  # ch11
  "../test262/test/suite/ch11/11.2/11.2.1/S11.2.1_A4_T9.js",   # Date
  "../test262/test/suite/ch11/11.4/11.4.1/11.4.1-4.a-8.js",    # JSON
  "../test262/test/suite/ch11/11.4/11.4.1/11.4.1-4.a-10.js",   # JSON
  "../test262/test/suite/ch11/11.2/11.2.1/S11.2.1_A4_T8.js",   # Math
  "../test262/test/suite/ch11/11.4/11.4.1/11.4.1-5-a-28-s.js", # RegExp
  "../test262/test/suite/ch11/11.8/11.8.6/S11.8.6_A5_T1.js",   # Error subclass
  "../test262/test/suite/ch11/11.8/11.8.6/S11.8.6_A6_T3.js",   # Error subclass
  "../test262/test/suite/ch11/11.4/11.4.3/S11.4.3_A3.2.js",    # RegExp
  # ch12
  "../test262/test/suite/ch12/12.2/12.2.1/12.2.1-20-s.js",  # indirect eval
  "../test262/test/suite/ch12/12.14/S12.14_A19_T1.js",      # Error subclass
  "../test262/test/suite/ch12/12.14/S12.14_A19_T2.js",      # Error subclass
  "../test262/test/suite/ch12/12.2/12.2.1/12.2.1-10-s.js",  # indirect eval
  "../test262/test/suite/ch12/12.2/12.2.1/12.2.1-21-s.js",  # indirect eval
  "../test262/test/suite/ch12/12.2/12.2.1/12.2.1-9-s.js",   # indirect eval
  "../test262/test/suite/ch12/12.2/12.2.1/12.2.1-22-s.js",  # indirect eval
  # ch14
  "../test262/test/suite/ch14/14.1/14.1-8-s.js",   # other directive
  "../test262/test/suite/ch14/14.1/14.1-14-s.js",  # other directive
]

def test(dir):
    if dir in not_fixed_tests:
      print("NOT FIXED TEST " + dir)
      return
    if str.find(dir, "intl402") >= 0:
      return
    print("start test " + dir)
    with open(dir) as f:
        test_file = f.read()
        has_error = str.find(test_file, " * @negative") >= 0
        comment_end = str.find(test_file, " */") + 3
        pure_script = test_file[comment_end:].strip()
        concat_file = ""
        if pure_script.startswith('"use strict";\n'):
            concat_file += '"use strict";\n'
        elif pure_script.startswith("'use strict';\n"):
            concat_file += "'use strict';\n"
        elif pure_script.startswith('"use strict";'):
            concat_file += '"use strict";'
        elif pure_script.startswith("'use strict';"):
            concat_file += "'use strict';"
        elif pure_script.startswith('"use strict"\n'):
            concat_file += '"use strict"\n'
        elif pure_script.startswith("'use strict'\n"):
            concat_file += "'use strict'\n"
        concat_file += harness + test_file

    tmp_path = os.path.join(current_path, "tmp.js")
    with open(tmp_path, "w") as f:
      f.write(concat_file)
    
    output = os.popen(f"{args.es} {tmp_path}").read()
    if has_error:
       if output == "" or not (str.find(output, "Uncaught ") >= 0 or str.find(output, "ParserError") >= 0):
          print("failed to raise Error")
    elif len(output) > 0:
        print(output)
        # exit()

test_path = args.test262_path

if test_path.endswith(".js"):
    test(test_path)
else:
    for (dirpath, dirnames, filenames) in os.walk(test_path):
        for filename in filenames:
            if filename.endswith('.js'):
                test(os.path.join(dirpath, filename))

