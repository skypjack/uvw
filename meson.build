project(
  'uvw',
  ['c', 'cpp'],
  version: '3.3.0',
  license: 'MIT',
  default_options: ['cpp_std=c++17'],
)

libuv_dep = dependency('libuv', version: '1.46.0')

sources = [
  'src/uvw/async.cpp',
  'src/uvw/check.cpp',
  'src/uvw/dns.cpp',
  'src/uvw/emitter.cpp',
  'src/uvw/fs.cpp',
  'src/uvw/fs_event.cpp',
  'src/uvw/fs_poll.cpp',
  'src/uvw/idle.cpp',
  'src/uvw/lib.cpp',
  'src/uvw/loop.cpp',
  'src/uvw/pipe.cpp',
  'src/uvw/poll.cpp',
  'src/uvw/prepare.cpp',
  'src/uvw/process.cpp',
  'src/uvw/signal.cpp',
  'src/uvw/stream.cpp',
  'src/uvw/tcp.cpp',
  'src/uvw/thread.cpp',
  'src/uvw/timer.cpp',
  'src/uvw/tty.cpp',
  'src/uvw/udp.cpp',
  'src/uvw/util.cpp',
  'src/uvw/work.cpp',
]

uvw_header_only_dep = declare_dependency(
  include_directories: 'src',
)

uvw_lib = library(
  'uvw', sources,
  include_directories: 'src',
  dependencies: [libuv_dep],
  cpp_args: ['-DUVW_AS_LIB'],
  install: true,
)

uvw_dep = declare_dependency(
  include_directories: ['src'],
  dependencies: [libuv_dep],
  link_with: [uvw_lib],
)

if meson.version().version_compare('>=0.54.0')
  meson.override_dependency('uvw', uvw_dep)
endif

pkg_mod = import('pkgconfig')
pkg_mod.generate(uvw_lib,
  name: 'uvw',
  filebase: 'uvw',
)
