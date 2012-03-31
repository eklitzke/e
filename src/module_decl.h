// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// This header file and the associated .cc file initialize all of the builtin
// modules. When a new builtin module is added, module_decl.cc before the module
// can actually be exported.

#ifndef SRC_MODULE_DECL_H_
#define SRC_MODULE_DECL_H_

namespace e {
// Initialize the set of builtin modules. This should be called before any
// JavaScript is loaded (including core.js).
void InitializeBuiltinModules();
}

#endif  // SRC_MODULE_DECL_H_
