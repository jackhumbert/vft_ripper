option(RED4EXT_USE_PCH "" ON)
option(RED4EXT_HEADER_ONLY "" ON)

add_subdirectory(deps/red4ext.sdk)
set_target_properties(RED4ext.SDK PROPERTIES FOLDER "Dependencies")

mark_as_advanced(
  RED4EXT_BUILD_EXAMPLES
  RED4EXT_USE_PCH
  RED4EXT_INSTALL
)
