{
  'targets': [
    {
      'target_name': 'e',
      'type': 'executable',
      'cflags': ['-g'],
      'conditions': [
        ['OS=="linux"', {
          'ldflags': [
            '-pthread',
          ],
          'libraries': [
            '-lgflags',
            '-lglog',
            '-lncurses',
            '-lv8',
          ]
        }]],
      'defines': [
        'DEFINE_DEBUG',
      ],
      'sources': [
        'src/main.cc',
        'src/buffer.cc',
        'src/curses_window.cc',
        'src/embeddable.cc',
        'src/js.cc',
        'src/keycode.cc',
        'src/state.cc',
      ]
    }
  ]
}
