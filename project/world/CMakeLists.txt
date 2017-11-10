# the exercise name is nothing else than the directory
get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

# these files are copied only once
file(GLOB TEXTURES
    grid/grid_texture.tga)
copy_files_once(${TEXTURES})
file(GLOB TEXTURES
    grid/grass_texture.tga)
copy_files_once(${TEXTURES})
file(GLOB TEXTURES
    grid/rock_texture.tga)
copy_files_once(${TEXTURES})
file(GLOB TEXTURES
    grid/snow_texture.tga)
copy_files_once(${TEXTURES})
file(GLOB TEXTURES
    grid/sand_texture.tga)
copy_files_once(${TEXTURES})
file(GLOB TEXTURES
    cube/sky_texture.tga)
copy_files_once(${TEXTURES})
file(GLOB TEXTURES
    water/grid_texture_2.tga)
copy_files_once(${TEXTURES})



# list all the files you want to copy everytime
# you build (ie, you want the shaders there)
file(GLOB SHADERS
  fractal/fractal_vshader.glsl
  fractal/fractal_fshader.glsl
  grid/grid_vshader.glsl
  grid/grid_fshader.glsl
  fractal/fractal_vshader.glsl
  fractal/fractal_fshader.glsl
  cube/cube_vshader.glsl
  cube/cube_fshader.glsl
  water/water_vshader.glsl
  water/water_fshader.glsl
  )
deploy_shaders_to_build_dir(${SHADERS})

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})
