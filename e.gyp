# -*- python -*-
{
  'target_defaults': {
    'type': 'executable',
    'cflags': ['-pedantic', '-Wall', '-std=c++11', '-g'],
    'conditions': [
       ['OS=="linux"', {
         'ldflags': [
           '-pthread',
          ],
          'libraries': [
            '-lboost_system',
            '-lboost_program_options',
            '<!@(pkg-config --libs-only-l libglog)',
            '<!@(pkg-config --libs-only-l icu-uc)',
            '<!@(pkg-config --libs-only-l ncursesw)',
            '-ltcmalloc',
            '-lunwind',
            '-lv8',
          ],
          'sources': [
            'src/assert.cc',
            'src/buffer.cc',
            'src/bundled_core.cc',
            'src/curses_window.cc',
            'src/curses_low_level.cc',
            'src/embeddable.cc',
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
            'src/mmap.cc',
            'src/module.cc',
            'src/module_decl.cc',
            'src/state.cc',
            'src/unicode.cc',
          ],
         'defines': [ 'USE_LINUX', ],
    }]],
    'defines': [ 'DEBUG', 'USE_CURSES', 'TAB_SIZE=4', ],
  },
  'targets': [
    {
      'target_name': 'e',
      'sources': [
        'src/main.cc',
      ],
    },
    {
      'target_name': 'test',
      'sources': [
        'src/tests/test.cc',
      ],
      'libraries': [
        '-lboost_unit_test_framework'
      ],
    },
  ],
}
