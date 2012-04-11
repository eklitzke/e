# -*- python -*-
{
  'target_defaults': {
    'type': 'executable',
    # cflags should be changed to -std=c++11 when ubuntu supports it
    'cflags': ['-pedantic', '-Wall', '-std=c++0x'],
    'conditions': [
       ['OS=="linux"', {
         'ldflags': [
           '-pthread',
          ],
         'libraries': ['-lunwind'],
         'defines': ['USE_LIBUNWIND', 'USE_LINUX'],
    }]],
    'defines': ['DEBUG', 'TAB_SIZE=4',],
    'libraries': [
      '-lboost_system',
      '-lboost_program_options',
      '<!@(pkg-config --libs-only-l ncursesw)',
      '-ltcmalloc',
      '-lv8',
    ],
    'sources': [
      'src/assert.cc',
      'src/buffer.cc',
      'src/curses_window.cc',
      'src/curses_low_level.cc',
      'src/event_listener.cc',
      'src/flags.cc',
      'src/io_service.cc',
      'src/js.cc',
      'src/js_curses.cc',
      'src/js_curses_window.cc',
      'src/js_errno.cc',
      'src/js_signal.cc',
      'src/js_sys.cc',
      'src/keycode.cc',
      'src/line.cc',
      'src/logging.cc',
      'src/mmap.cc',
      'src/module.cc',
      'src/module_decl.cc',
      'src/state.cc',
      'src/timer.cc',
    ],
  },
  'targets': [
    {
      'target_name': 'e',
      'cflags': ['-g'],
      'sources': [
        'src/main.cc',
      ],
    },
    {
      'target_name': 'opt',
      'cflags': ['-Os'],
      'defines': ['OPTIMIZED_BUILD'],
      'libraries': ['<!@(pkg-config --libs-only-l liblzma)'],
      'sources': [
        'src/bundled_core.cc',
        'src/main.cc',
      ],
    },
    {
      'target_name': 'test',
      'cflags': ['-g', '-O0'],
      'sources': [
        'src/tests/test.cc',
      ],
      'libraries': [
        '-lboost_unit_test_framework'
      ],
    },
  ],
}
