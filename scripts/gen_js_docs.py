#!/usr/bin/env python

import cgi
import jinja2
import optparse
import os
import re
import sys

source_regex = re.compile(r'\.(cc|h)$')
command_regex = re.compile(r'^\s*// @(?P<comm>[-a-zA-Z_]+)(?P<param>\[[-a-zA-Z_]+\])?:\s+(?P<data>.*)\s*$')
cont_regex = re.compile(r'^\s*//\s+(.*)\s*$')

class Documentation(object):

    def __init__(self):
        self.functions = []
        self.prototypes = []

    def add_function(self, func):
        self.functions.append(func)
    
    def add_prototype(self, proto):
        self.prototypes.append(proto)

    def _add_html(self, tmpl, *bind_vals):
        bind_vals = tuple(cgi.escape(v) for v in bind_vals)
        self.html_out.append(tmpl % bind_vals)

    def render(self, static_dir, outfile):
        loader = jinja2.FileSystemLoader(os.path.join(
                os.path.dirname(__file__), 'templates'))
        env = jinja2.Environment(loader=loader)
        template = env.get_template('jsdoc.html')
        context = {'prototypes': sorted(self.prototypes, key=lambda x: x.name),
                   'functions': sorted(self.functions, key=lambda x: x.name),
                   'static_dir': static_dir.rstrip('/')}
 
        for chunk in template.generate(**context):
            outfile.write(chunk)

doc = Documentation()

class JSVar(object):

    def __init__(self, name, type='', description=None):
        self.name = name
        self.type = type
        self._description = description or []

    @property
    def description(self):
        val = ' '.join(self._description)
        while True:
            c = val.count('`')
            if c == 0:
                break
            elif c % 2 == 0:
                val = val.replace('`', '<span class="code">', 1)
            else:
                val = val.replace('`', '</span>', 1)
        return val

    def update_description(self, desc):
        self._description.append(desc)

class JSFunction(JSVar):

    def __init__(self, name):
        super(JSFunction, self).__init__(name)
        self.arguments = []
        self.is_accessor = False
        self.return_type = ''
        self.return_val = ''

    def add_argument(self, arg):
        self.arguments.append(arg)

    def add_returns(self, data):
        if data.startswith('#'):
            type, val = data.split(' ', 1)
            self.return_type = type[1:]
            self.return_val = val
        else:
            self.return_val = data

class JSPrototype(JSVar):

    _prototypes = []

    def __init__(self, name):
        super(JSPrototype, self).__init__(name)
        self.methods = []

    @classmethod
    def get_prototype(cls, name):
        for p in cls._prototypes:
            if p.name == name:
                return p
        proto = cls(name)
        doc.add_prototype(proto)
        return proto
        
    def add_method(self, method):
        assert not any(m.name == method.name for m in self.methods), '%s.%s' % (self.name, method.name)
        self.methods.append(method)

class ParseState(object):

    def __init__(self, src_name):
        self.src_name = src_name
        self.groups = []
        self.current_group = []
        self.parse_multiline = False

    def command_groups(self, lines):
        """iterate over all of the lines, yielding groups of (command, data)"""
        for line in lines:
            match = command_regex.match(line)
            if match:
                if self.current_group:
                    yield self.current_group
                groupdict = match.groupdict()
                comm = groupdict['comm']
                param = groupdict['param']
                if param:
                    param = param[1:-1]
                data = groupdict['data']
                self.current_group = (comm, param, data)
                self.parse_multiline = True
            elif self.parse_multiline:
                match = cont_regex.match(line)
                if cont_regex.match(line):
                    new_data, = match.groups()
                    if new_data:
                        name, param, data = self.current_group
                        data += ' ' + new_data
                        self.current_group = (name, param, data)
                    else:
                        self.parse_multiline = False
                else:
                    self.parse_multiline = False
        if self.current_group:
            yield self.current_group
            self.current_group = []

    def get_toplevel(self, lines):
        proto = None
        func = None
        param = None
        cur_type = ''
        for command, par, data in self.command_groups(lines):
            if command == 'class':
                cur_type = 'class'
                if data in ('null', 'none'):
                    proto = None
                else:
                    proto = JSPrototype.get_prototype(data)
            elif command in ('method', 'accessor'):
                assert proto is not None
                cur_type = 'func'
                func = JSFunction(data)
                proto.add_method(func)
                if command == 'accessor':
                    func.is_accessor = True
            elif command == 'param':
                assert func is not None
                assert par
                if data.startswith('#'):
                    vartype, data = data.split(' ', 1)
                    vartype = vartype[1:]
                    param = JSVar(par, vartype)
                else:
                    param = JSVar(par)
                param.update_description(data)
                if cur_type == 'class':
                    proto.add_argument(param)
                elif cur_type == 'func':
                    func.add_argument(param)
                else:
                    raise ValueError('Unknown curtype %r' % (cur_type,))
            elif command == 'description':
                if cur_type == 'class':
                    proto.update_description(data)
                elif cur_type == 'func':
                    func.update_description(data)
                else:
                    assert False
            elif command == 'returns':
                assert cur_type == 'func'
                func.add_returns(data)
            else:
                raise ValueError('Unknown command %r' % (command,))

def index_file(src_file):
    state = ParseState(src_file)
    with open(src_file) as f:
        state.get_toplevel(f)

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-d', '--directory', default='src/', help='The directory to scan')
    parser.add_option('-o', '--output', default=None, help='The output file')
    parser.add_option('--static-dir', default='../static/', help='The static directory file')
    opts, args = parser.parse_args()

    for f in sorted(args):
        index_file(f)

    if opts.output:
        outdir = os.path.dirname(opts.output)
        if not os.path.exists(outdir):
            os.makedirs(outdir)
        with open(opts.output, 'w') as f:
            doc.render(opts.static_dir, f)
    else:
        doc.render(opts.static_dir, sys.stdout)
