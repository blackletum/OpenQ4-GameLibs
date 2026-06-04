py = import('python').find_installation(required : true)
cpp = meson.get_compiler('cpp')

if host_machine.system() != 'windows'
  error('openQ4-GameLibs Meson build currently targets Windows only.')
endif

if cpp.get_id() != 'msvc'
  error('openQ4-GameLibs Meson build requires MSVC cl.exe.')
endif

if host_machine.cpu_family() != 'x86'
  error('openQ4-GameLibs Meson build requires an x86 (32-bit) toolchain. Use an x86 Visual Studio environment.')
endif

game_arch = host_machine.cpu_family()
sp_module_name = 'game-sp_' + game_arch
mp_module_name = 'game-mp_' + game_arch

source_lister = join_paths(meson.current_source_dir(), 'buildscripts', 'list_sources.py')
source_root = meson.current_source_dir()
release_like = ['release', 'debugoptimized', 'minsize'].contains(get_option('buildtype'))
source_inc = include_directories('.')

common_cpp_args = [
  '/W4',
  '/FS',
  '/source-charset:windows-1252',
  '/execution-charset:windows-1252',
  '/Zc:strictStrings-',
  '/wd4091',
  '/wd4456',
  '/wd4457',
  '/wd4458',
  '/wd4459',
  '/wd4595',
  '/wd4996',
  '/wd4702',
  '/wd4740',
  '/wd4750',
]

game_forced_include = '/FI' + join_paths(meson.current_source_dir(), 'idlib', 'precompiled.h')

mssdk_include = get_option('mssdk_include')
if mssdk_include != ''
  common_cpp_args += ['/I' + join_paths(meson.current_source_dir(), mssdk_include)]
endif

common_defines = [
  'Q4SDK',
  '_WINDOWS',
  'WIN32',
  'USE_OPENAL',
]

if release_like
  common_defines += ['NDEBUG']
else
  common_defines += ['_DEBUG']
endif

if get_option('inline_debug') and not release_like
  common_defines += ['_INLINEDEBUG']
  common_cpp_args += ['/Ob1']
endif

common_define_args = []
foreach define : common_defines
  common_define_args += '/D' + define
endforeach

idlib_source_output = run_command(
  py,
  source_lister,
  source_root,
  'idlib',
  'idlib/math/Simd_AltiVec.cpp',
  check : true,
).stdout().strip().split('\n')

idlib_sources = []
foreach source_line : idlib_source_output
  rel = source_line.strip()
  if rel != ''
    idlib_sources += files(rel)
  endif
endforeach

idlib = static_library(
  'idLib',
  idlib_sources,
  include_directories : source_inc,
  cpp_args : common_cpp_args + common_define_args + ['/DGAME_DLL'],
  install : false,
)

module_link_args = [
  '/FIXED:NO',
  '/BASE:0x20000000',
  '/LARGEADDRESSAWARE',
]

if release_like
  module_link_args += [
    '/STACK:4194304',
    '/OPT:REF',
    '/OPT:ICF',
    '/MAP',
  ]
else
  module_link_args += ['/STACK:16000000,16000000']
endif

if get_option('build_spgame')
  game_source_output = run_command(
    py,
    source_lister,
    source_root,
    'game',
    'game/Callbacks.cpp',
    'game/gamesys/Callbacks.cpp',
    check : true,
  ).stdout().strip().split('\n')

  game_sources = []
  foreach source_line : game_source_output
    rel = source_line.strip()
    if rel != ''
      game_sources += files(rel)
    endif
  endforeach

  shared_module(
    sp_module_name,
    game_sources,
    include_directories : source_inc,
    link_with : idlib,
    cpp_args : common_cpp_args + common_define_args + ['/DGAME_DLL', game_forced_include],
    link_args : module_link_args,
    vs_module_defs : 'game/game.def',
    name_prefix : '',
    install : false,
  )
endif

if get_option('build_mpgame')
  mpgame_source_output = run_command(
    py,
    source_lister,
    source_root,
    'mpgame',
    'mpgame/Callbacks.cpp',
    'mpgame/gamesys/Callbacks.cpp',
    check : true,
  ).stdout().strip().split('\n')

  mpgame_sources = []
  foreach source_line : mpgame_source_output
    rel = source_line.strip()
    if rel != ''
      mpgame_sources += files(rel)
    endif
  endforeach

  shared_module(
    mp_module_name,
    mpgame_sources,
    include_directories : source_inc,
    link_with : idlib,
    cpp_args : common_cpp_args + common_define_args + [
      '/DGAME_DLL',
      '/DGAME_MPAPI',
      game_forced_include,
    ],
    link_args : module_link_args,
    vs_module_defs : 'mpgame/mpgame.def',
    name_prefix : '',
    install : false,
  )
endif

summary(
  {
    'Build Type': get_option('buildtype'),
    'Build SP Game': get_option('build_spgame'),
    'Build MP Game': get_option('build_mpgame'),
    'Inline Debug': get_option('inline_debug') and not release_like,
    'Legacy Include': mssdk_include == '' ? '(disabled)' : mssdk_include,
  },
  section : 'openQ4-GameLibs',
)
