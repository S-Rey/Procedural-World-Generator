#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>

class Grid {

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint texture_id_;                     // texture ID
        GLuint texture_perlin_id_;              // texture for Perlin
        GLuint texture_id_2;                    // texture ID
        GLuint texture_id_snow;                    // texture ID
        GLuint texture_id_rock;                    // texture ID
        GLuint texture_id_sand;                    // texture ID
        GLuint texture_id_grass;                    // texture ID
        GLuint num_indices_;                    // number of vertices to render
        GLuint MVP_id_;                         // model, view, proj matrix ID
        GLuint projection_id_;
        GLuint view_id_;
        GLuint model_id_;

    public:
        void Init(GLuint tex_perlin = -1) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("grid_vshader.glsl",
                                                  "grid_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates and indices
            {
                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;
                // TODO 5: make a triangle grid with dimension 100x100.
                // always two subsequent entries in 'vertices' form a 2D vertex position.
                int grid_dim = 512;
                float step = float(2.0 / float(grid_dim));
                std::cout << step << std::endl;

                // the given code below are the vertices for a simple quad.
                // your grid should have the same dimension as that quad, i.e.,
                // reach from [-1, -1] to [1, 1].

                // vertex position of the triangles.

                int index(0);

                for (float x = -1.0f; x < 1.0f; x+= step)
                {
                    for (float y = -1.0f; y < 1.0f; y+= step)
                    {
                        vertices.push_back(x);          vertices.push_back(y + step);
                        vertices.push_back(x + step);   vertices.push_back(y + step);
                        vertices.push_back(x + step);   vertices.push_back(y);
                        vertices.push_back(x);          vertices.push_back(y);

                        indices.push_back(index + 0);
                        indices.push_back(index + 1);
                        indices.push_back(index + 2);

                        indices.push_back(index + 2);
                        indices.push_back(index + 3);
                        indices.push_back(index + 0);

                        index += 4;
                    }

                }



                num_indices_ = indices.size();

                // position buffer
                glGenBuffers(1, &vertex_buffer_object_position_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_position_);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                             &vertices[0], GL_STATIC_DRAW);

                // vertex indices
                glGenBuffers(1, &vertex_buffer_object_index_);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer_object_index_);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                             &indices[0], GL_STATIC_DRAW);

                // position shader attribute
                GLuint loc_position = glGetAttribLocation(program_id_, "position");
                glEnableVertexAttribArray(loc_position);
                glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }


            // load texture
            {
                int width;
                int height;
                int nb_component;
                string filename = "grid_texture.tga";
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);
                unsigned char* image = stbi_load(filename.c_str(), &width,
                                                 &height, &nb_component, 0);

                if(image == nullptr) {
                    throw(string("Failed to load texture"));
                }

                glGenTextures(1, &texture_id_);
                glBindTexture(GL_TEXTURE_2D, texture_id_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                if(nb_component == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image);
                } else if(nb_component == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image);
                }


                texture_perlin_id_ = (tex_perlin==-1)? texture_id_ : tex_perlin;

                // texture uniforms
                GLuint tex_id = glGetUniformLocation(program_id_, "tex");
                glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

                GLuint tex_perlin_id = glGetUniformLocation(program_id_, "tex_perlin");
                glUniform1i(tex_perlin_id, 1 /*GL_TEXTURE1*/);
                
                
                int width_snow;
                int height_snow;
                int nb_component_snow;
                string filename_snow = "snow_texture.tga";
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);
                unsigned char* image_snow = stbi_load(filename_snow.c_str(), &width_snow,
                                                 &height_snow, &nb_component_snow, 0);
                
                if(image_snow == nullptr) {
                    throw(string("Failed to load texture"));
                }
                
                glGenTextures(1, &texture_id_snow);
                glBindTexture(GL_TEXTURE_2D, texture_id_snow);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                if(nb_component_snow == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_snow, height_snow, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image_snow);
                } else if(nb_component_snow == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_snow, height_snow, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image_snow);
                }
                
                
                
                // texture uniforms
                GLuint tex_id_snow = glGetUniformLocation(program_id_, "snow");
                glUniform1i(tex_id_snow, 2 /*GL_TEXTURE2*/);

                
                int width_rock;
                int height_rock;
                int nb_component_rock;
                string filename_rock = "rock_texture.tga";
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);
                unsigned char* image_rock = stbi_load(filename_rock.c_str(), &width_rock,
                                                 &height_rock, &nb_component_rock, 0);
                
                if(image_rock == nullptr) {
                    throw(string("Failed to load texture"));
                }
                
                glGenTextures(1, &texture_id_rock);
                glBindTexture(GL_TEXTURE_2D, texture_id_rock);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                if(nb_component_rock == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_rock, height_rock, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image_rock);
                } else if(nb_component_rock == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_rock, height_rock, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image_rock);
                }
                
                // texture uniforms
                GLuint tex_id_rock = glGetUniformLocation(program_id_, "rock");
                glUniform1i(tex_id_rock, 3 /*GL_TEXTURE3*/);

                
                int width_grass;
                int height_grass;
                int nb_component_grass;
                string filename_grass = "grass_texture.tga";
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);
                unsigned char* image_grass = stbi_load(filename_grass.c_str(), &width_grass,
                                                 &height_grass, &nb_component_grass, 0);
                
                if(image_grass == nullptr) {
                    throw(string("Failed to load texture"));
                }
                
                glGenTextures(1, &texture_id_grass);
                glBindTexture(GL_TEXTURE_2D, texture_id_grass);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                if(nb_component_grass == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_grass, height_grass, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image_grass);
                } else if(nb_component_grass == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_grass, height_grass, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image_grass);
                }
                
                // texture uniforms
                GLuint tex_id_grass = glGetUniformLocation(program_id_, "grass");
                glUniform1i(tex_id_grass, 4 /*GL_TEXTURE4*/);
          
                
                int width_sand;
                int height_sand;
                int nb_component_sand;
                string filename_sand = "sand_texture.tga";
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);
                unsigned char* image_sand = stbi_load(filename_sand.c_str(), &width_sand,
                                                 &height_sand, &nb_component_sand, 0);
                
                if(image_sand == nullptr) {
                    throw(string("Failed to load texture"));
                }
                
                glGenTextures(1, &texture_id_sand);
                glBindTexture(GL_TEXTURE_2D, texture_id_sand);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                if(nb_component_sand == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_sand, height_sand, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image_sand);
                } else if(nb_component_sand == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_sand, height_sand, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image_sand);
                }
                
                // texture uniforms
                GLuint tex_id_sand = glGetUniformLocation(program_id_, "sand");
                glUniform1i(tex_id_sand, 5 /*GL_TEXTURE5*/);

                
                
                // cleanup
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_1D, 1);
                stbi_image_free(image);
                glBindTexture(GL_TEXTURE_2D, 2);
                stbi_image_free(image_snow);
                glBindTexture(GL_TEXTURE_2D, 3);
                stbi_image_free(image_rock);
                glBindTexture(GL_TEXTURE_2D, 4);
                stbi_image_free(image_grass);
                glBindTexture(GL_TEXTURE_2D, 5);
                stbi_image_free(image_sand);

            }

            // other uniforms
            MVP_id_ = glGetUniformLocation(program_id_, "MVP");
            model_id_ = glGetUniformLocation(program_id_, "model");
            projection_id_ = glGetUniformLocation(program_id_, "projection");
            view_id_ = glGetUniformLocation(program_id_, "view");


            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);
            glDeleteTextures(1, &texture_id_);
            glDeleteTextures(1, &texture_perlin_id_);
            glDeleteTextures(1, &texture_id_snow);
            glDeleteTextures(1, &texture_id_rock);
            glDeleteTextures(1, &texture_id_grass);
            glDeleteTextures(1, &texture_id_sand);
            
        }

        void Draw(float time, const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX, bool d = false ) {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id_);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture_perlin_id_);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture_id_snow);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, texture_id_rock);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, texture_id_grass);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, texture_id_sand);


            // setup

            glm::mat4 MVP = projection*view*model;
            glUniformMatrix4fv(MVP_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(model_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            glUniformMatrix4fv(view_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(view));
            glUniformMatrix4fv(projection_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(projection));



            // pass the current time stamp to the shader.
            glUniform1f(glGetUniformLocation(program_id_, "time"), time);
            glUniform1f(glGetUniformLocation(program_id_, "d"), d);

            // draw
            glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
