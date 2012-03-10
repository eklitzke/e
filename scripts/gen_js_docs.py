#!/usr/bin/env python

import cgi
import optparse
import os
import re

source_regex = re.compile(r'\.(cc|h)$')
command_regex = re.compile(r'^\s*// @(?P<comm>[-a-zA-Z_]+)(?P<param>\[[-a-zA-Z_]+\])?:\s+(?P<data>.*)\s*$')
cont_regex = re.compile(r'^\s*//\s+(.*)\s*$')

def emit_html(tmpl, *bind_vals):
    bind_vals = tuple(cgi.escape(v) for v in bind_vals)
    return tmpl % bind_vals

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

    def to_html(self):
        out = []
        for p in self.prototypes:
            out.append(p.to_html())
        for f in self.functions:
            out.append(f.to_html())
        return '\n'.join(out)

doc = Documentation()

class JSVar(object):

    def __init__(self, name, type='', description=[]):
        self.name = name
        self.type = type
        self._description = description

    @property
    def description(self):
        return ' '.join(self._description)

    def update_description(self, desc):
        self._description.append(desc)

class JSFunction(JSVar):

    def __init__(self, name):
        super(JSFunction, self).__init__(name)
        self.arguments = []

    def add_argument(self, arg):
        self.arguments.add(arg)

    def to_html(self):
        out = []
        out.append(emit_html('<div class="function">'))
        out.append(emit_html('<div class="function_name">%s</div>', self.name))
        out.append(emit_html('</div> <!-- .function -->'))
        return '\n'.join(out)

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
        self.methods.append(method)

    def to_html(self):
        out = []
        out.append(emit_html('<div class="prototype">'))
        out.append(emit_html('<div class="prototype_name">%s</div>', self.name))
        for meth in self.methods:
            out.append(meth.to_html())
        out.append(emit_html('</div> <!-- .prototype -->'))
        return '\n'.join(out)

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
            elif command == 'method':
                assert proto is not None
                cur_type = 'func'
                func = JSFunction(data)
                proto.add_method(func)
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
            elif command == 'description':
                if cur_type == 'class':
                    proto.update_description(data)
                elif cur_type == 'func':
                    func.update_description(data)
                else:
                    assert False
            else:
                raise ValueError('Unknown command %r' % (command,))

def index_file(src_file):
    state = ParseState(src_file)
    with open(src_file) as f:
        state.get_toplevel(f)
    
if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-d', '--directory', default='src/', help='The directory to scan')
    opts, args = parser.parse_args()

    for dirpath, dirnames, filenames in os.walk(opts.directory):
        for f in (f for f in filenames if source_regex.search(f)):
            index_file(os.path.join(dirpath, f))
                                 
    print doc.to_html()
