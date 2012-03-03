# -*- python -*-
{
  'target_defaults': {
    'type': 'executable',
    'cflags': ['-Wall', '-std=c++11'],
    'conditions': [
       ['OS=="linux"', {
         'ldflags': [
           '-pthread',
          ],
          'libraries': [
            '-lboost_system',
            '-lboost_program_options',
            '-lglog',
            '-lncurses',
            '-lv8',
          ]
    }]],
    'sources': [
        'src/main.cc',
        'src/buffer.cc',
        'src/curses_window.cc',
        'src/embeddable.cc',
        'src/js.cc',
        'src/js_curses.cc',
        'src/keycode.cc',
        'src/line.cc',
        'src/list_api.cc',
        'src/state.cc',
    ],
    'defines': [ 'USE_CURSES', ],
  },
  'targets': [
    {
      'target_name': 'e',
      'cflags': ['-g'],
      'defines': [ 'DEBUG', ],
    },
  ],
}
