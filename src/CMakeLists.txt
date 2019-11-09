cmake_minimum_required(VERSION 3.5)

add_library(${BINARY_NAME} SHARED
  cg_consolecmds.c
  cg_cvar.c
  cg_draw.c
  cg_entity_states.c
  cg_filter_sounds.c
  cg_gl.c
  cg_hud.c
  cg_init.c
  cg_main.c
  cg_rl.c
  cg_syscall.c
  cg_timer_hud.c
  cg_trigger_vis.c
  cg_utils.c
  cg_vm.c
  q_math.c)

set_target_properties(${BINARY_NAME} PROPERTIES
  LINK_FLAGS_RELEASE -s
  PREFIX "")

target_include_directories(${BINARY_NAME}
  PRIVATE ../include)

target_link_libraries(${BINARY_NAME}
  PUBLIC m)

install(TARGETS ${BINARY_NAME} DESTINATION defrag)
