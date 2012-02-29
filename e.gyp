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
        'src/embeddable.cc',
        'src/js.cc',
        'src/js_curses.cc',
        'src/keycode.cc',
        'src/state.cc',
        'src/termios_window.cc',
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
