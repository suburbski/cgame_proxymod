cmake_minimum_required(VERSION 3.5)

add_library(${BINARY_NAME} SHARED
  bg_misc.c
  cg_ammo.c
  cg_consolecmds.c
  cg_cvar.c
  cg_draw.c
  cg_entity.c
  cg_gl.c
  cg_hud.c
  cg_jump.c
  cg_main.c
  cg_marks.c
  cg_rl.c
  cg_syscall.c
  cg_timer.c
  cg_trig.c
  cg_utils.c
  cg_view.c
  cg_vm.c
  g_missile.c
  g_weapon.c
  q_math.c
  q_shared.c)

set_target_properties(${BINARY_NAME} PROPERTIES
  LINK_FLAGS_RELEASE -s
  PREFIX "")

target_include_directories(${BINARY_NAME}
  PRIVATE ../include)

target_link_libraries(${BINARY_NAME}
  PUBLIC m)

install(TARGETS ${BINARY_NAME} DESTINATION defrag)
