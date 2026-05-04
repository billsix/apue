# conf.c and options.c are generated at build time by awk scripts that
# read paired .sym files. The make build also filters comment lines
# from `*-lim.sym` / `*-opt.sym` source files into shorter `.sym` files
# before invoking awk; we do the same in scripts/gen_standards_c.sh.

gen_standards = find_program(meson.project_source_root() / 'scripts' / 'gen_standards_c.sh')

conf_c = custom_target('conf_c',
  output : 'conf.c',
  input : files(
    'makeconf.awk',
    'sysconf-lim.sym',
    'pathconf-lim.sym',
  ),
  command : [
    gen_standards,
    '@OUTPUT@',
    '@INPUT0@',
    '@INPUT1@', 'sysconf.sym',
    '@INPUT2@', 'pathconf.sym',
  ],
)

options_c = custom_target('options_c',
  output : 'options.c',
  input : files(
    'makeopt.awk',
    'sysconf-opt.sym',
    'pathconf-opt.sym',
  ),
  command : [
    gen_standards,
    '@OUTPUT@',
    '@INPUT0@',
    '@INPUT1@', 'sysopt.sym',
    '@INPUT2@', 'pathopt.sym',
  ],
)

executable('conf', conf_c, dependencies : libapue_dep)
executable('options', options_c, dependencies : libapue_dep)
