#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <random>
#include <cmath>


typedef struct camera_variables {
    glm::vec3 pos;
    glm::vec3 look;
    glm::vec3 up;
} camera_variables;


class Bezier
{
    private:
        std::vector<camera_variables> cam_table;

    public:
        bool on = true;
        bool run = false;
        float running_time = 5.0; // 5 sec
        float begin_time;
        float end_time;
        Bezier()
        {
            cam_table = std::vector<camera_variables>();
        }


        void add(glm::vec3 pos, glm::vec3 look, glm::vec3 up)
        {
            camera_variables cam = {pos, look, up};
            cam_table.push_back(cam);
        }


        void clear()
        {
            cam_table.clear();
        }


        int size()
        {
            return cam_table.size();
        }


        struct camera_variables compute(float t)
        {
            camera_variables c = {
                glm::vec3(0.0, 0.0, 0.0),
                glm::vec3(0.0, 0.0, 0.0),
                glm::vec3(0.0, 0.0, 0.0)
            };
            if (t < 0 || t > 1 || (cam_table.size() <= 2))
                    return c;

            float n = cam_table.size()-1;
            float n_fact = fact(n);
            for(int i = 0; i < cam_table.size(); ++i) {
                float bin = n_fact / (fact(i) * fact(n-i)) * pow(t, i) * pow(1-t, n-i);
                c.pos += bin * cam_table[i].pos;
                c.look += bin * cam_table[i].look;
                c.up += bin * cam_table[i].up;

            }
            return c;
        }


        float fact(int n)
        {
            if (n <= 0)
                return 1;
            return n * fact(n-1);
        }

        bool enough_measures()
        {
            return (cam_table.size()>2);
        }

};
