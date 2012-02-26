# -*- python -*-
{
  'target_defaults': {
    'type': 'executable',
    'cflags': ['-Wall'],
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
            #'-ltermcap',
            '-lv8',
          ]
    }]],
    'sources': [
        'src/main.cc',
        'src/buffer.cc',
        #'src/curses_window.cc',
        'src/embeddable.cc',
        'src/js.cc',
        'src/keycode.cc',
        'src/state.cc',
        'src/termios_window.cc',
    ]
  },
  'targets': [
    {
      'target_name': 'e',
      'cflags': ['-g'],
      'defines': [ 'DEBUG', ],
    },
  ],
}
