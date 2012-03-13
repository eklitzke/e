"""Simple script to get the mouse constants from curses.h"""

import optparse
import os
import re
import subprocess
import tempfile

#button_macro = re.compile(r'^#define\s+((BUTTON\d?_|REPORT_MOUSE_)[_A-Z]+)\s+')
button_macro = re.compile(r'^#define\s+(BUTTON\d?_[_A-Z]+)\s+')
undeclared = re.compile(r"'([A-Z_0-9]+)' undeclared")

hdr = """#include <stdio.h>
#include <%(header)s>

int main(int argc, char **argv) {
"""

tail = """  return 0;
}
"""

def emit_file(input_file, filename, keys):
    with open(filename, 'w') as b:
        b.write(hdr % {'header': os.path.basename(input_file)})
        for k in sorted(keys):
            b.write('  printf("%s %%d\\n", %s);\n' % (k, k));
        b.write(tail)

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-f', '--file', default='/usr/include/curses.h', help='path to curses header')
    opts, args = parser.parse_args()
    keys = set()
    with open(opts.file) as input_file:
        for line in input_file:
            m = button_macro.match(line)
            if m:
                macro_name = m.groups()[0]
                keys.add(macro_name)

    tempdir = os.environ.get('TEMPDIR', '/tmp')
    fd1, c_name = tempfile.mkstemp(suffix='.c', dir=tempdir)
    fd2, a_name = tempfile.mkstemp(dir=tempdir)
    try:
        os.close(fd1)
        os.close(fd2)
        emit_file(opts.file, c_name, keys)

        os.environ['LANG'] = 'C'
        p = subprocess.Popen(['gcc', c_name, '-o', a_name], stderr=subprocess.PIPE)
        out, err = p.communicate()
        if p.returncode != 0:
            for line in err.split('\n'):
                m = undeclared.search(line)
                if m:
                    keys.remove(m.groups()[0])
            emit_file(opts.file, c_name, keys)
            p = subprocess.Popen(['gcc', c_name, '-o', a_name], stderr=subprocess.PIPE)
            out, err = p.communicate()
            assert p.returncode == 0
        for k in sorted(keys):
            print ''
            print '  // @accessor: %s' % (k,)
            print '  NEW_INTEGER(curses, %s);' % (k,)
    finally:
        os.unlink(c_name)
        os.unlink(a_name)

