#include "oglwrap_example.hpp"
#define GLM_FORCE_RADIANS
#include <boost/python.hpp>

#include <oglwrap/oglwrap.h>
#include "custom_shape.h"
#include <oglwrap/shapes/cube_shape.h>
#include <glm/gtc/matrix_transform.hpp>
#include <lodepng.h>
#include <utility>
#include <numeric>
#include <boost/python/stl_iterator.hpp>

#include "player.h"

void MeshGen(std::vector<float> const& pts, std::vector<unsigned> const& inds, std::vector<float> const& normals,
         std::vector<MeshPoint>& points, std::vector<unsigned>& inds_out)
{
    auto pt_itr = pts.begin(), nm_itr = normals.begin();
    for(int i = 0; i < pts.size() / 3; ++i)
    {
        points.emplace_back(*pt_itr++, *pt_itr++, *pt_itr++, *nm_itr++, *nm_itr++, *nm_itr++, 0, 0, 9);
    }

    int cur_size = inds_out.size();
    inds_out.resize(cur_size + inds.size());
    std::copy(inds.begin(), inds.end(), inds_out.begin() + cur_size);
}

class Graphics : public OglwrapExample {
    private:
        gl::Program prog_;
        gl::Texture2D tex_;

        glm::vec2 camAng = {M_PI_2, 0};
        glm::vec3 camForward = {0, 0, 1};

        float togglePrevTime = -1;
        bool centerCam = true;
        float centerPrevTime = -1;

    public:

        Mesh myMesh;
        std::vector<MeshPoint> myMeshPoints_;
        std::vector<unsigned> myMeshInds_;
        glm::vec2 myMeshAng = {0, 0};

        Player player_;

        Graphics (std::vector<float> const& myPts, std::vector<unsigned> const& myInds, std::vector<float> const& myNormals,
                int prim_type)
            : player_(glm::vec3(0,0,-1))
        {
            MeshGen(myPts, myInds, myNormals, myMeshPoints_, myMeshInds_);
            myMesh.Set(&myMeshPoints_, &myMeshInds_);
            myMesh.SetPrimType((gl::PrimType)prim_type);

            // We need to add a few more lines to the shaders
            gl::ShaderSource vs_source;
            gl::ShaderSource fs_source;

            if(prim_type < 4)
            {
                vs_source.loadFromFile("vert_flat.glsl");
                fs_source.loadFromFile("frag_flat.glsl");
            }
            else
            {
                vs_source.loadFromFile("vert_surface_shade.glsl");
                fs_source.loadFromFile("frag_surface_shade.glsl");
            }

            gl::Shader vs(gl::kVertexShader, vs_source);
            gl::Shader fs(gl::kFragmentShader, fs_source);

            gl::PointSize(4);

            // Create a shader program
            prog_.attachShader(vs);
            prog_.attachShader(fs);
            prog_.link();
            gl::Use(prog_);

            // Bind the attribute locations
            (prog_ | "inPos").bindLocation(0);
            (prog_ | "inNormal").bindLocation(1);
            (prog_ | "inUV").bindLocation(2);

            gl::Enable(gl::kDepthTest);
            // Set the clear color
            gl::ClearColor(1.0, 1.0, 1.0, 1.0f);

            // Disable cursor.
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window_, GLFW_STICKY_KEYS, true);

            // Set callbacks.
            glfwSetMouseButtonCallback(window_, MouseDiscreteCallback);

            // Set user pointer. This is used in the callbacks, which are static.
            glfwSetWindowUserPointer(window_, this);

            // Enable alpha blending
            gl::Enable(gl::kBlend);
            gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha);
        }

        virtual bool Render() override {
            float t = glfwGetTime();

            gl::Clear().Color().Depth();
            glfwPollEvents();

            // Player position update.
            player_.Integrate(0.01);
            glm::vec3 pos = player_.GetPos();

            glm::vec3 lookOrigin = pos + camForward;
            if(centerCam)
                lookOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::mat4 camera_mat = glm::lookAt(player_.GetPos(), lookOrigin, glm::vec3{0.0f, 1.0f, 0.0f});
            glm::mat4 model_mat = glm::mat4x4(1.0f);
            glm::mat4 proj_mat = glm::perspectiveFov<float>(M_PI/2.0, kScreenWidth, kScreenHeight, 0.0001f, 50);

            if(myMesh.GetPrimType() == gl::PrimType::kTriangles) // Mesh needs shading.
            {
                glm::vec3 lightPos1 = player_.GetPos();
                gl::Uniform<glm::vec3>(prog_, "lightPos1") = lightPos1;
            }

            // Render my model.
            model_mat = glm::rotate(glm::mat4(1.0f), glm::radians(myMeshAng[0]), glm::vec3(0.0f, 1.0f, 0.0f));
            gl::Uniform<glm::mat4>(prog_, "mvp") = proj_mat * camera_mat * model_mat;
            myMesh.Render();

            HandleKeys();
            glfwSwapBuffers(window_);
//            HandleMouse();
            return !glfwWindowShouldClose(window_);
        }


    protected:
        // Only called when mouse events (release/myss) happen. This is good when you 
        // don't want to register multiple clicks when the user clicks once.
        static void MouseDiscreteCallback(GLFWwindow* window, int button, int action, int mods)
        {
//            Graphics* g = static_cast<Graphics*>(glfwGetWindowUserPointer(window));
        }

        void HandleMouse()
        {
            double xpos, ypos;
            glfwGetCursorPos(window_, &xpos, &ypos);
            glfwSetCursorPos(window_, kScreenWidth / 2., kScreenHeight / 2.);

            // Convert xpos, ypos from screen coordinates to [-1,1] coordinates.
            glm::vec2 wpos = {(float)xpos, (float)ypos};
            wpos = glm::vec2(2) * wpos / glm::vec2(kScreenWidth, -kScreenHeight);
            wpos += glm::vec2(-1, 1);

            // Update camera angle. Notice that we restrict the y angle 
            // because we don't want to flip the camera by crossing pi/2 or -pi/2.
            camAng.x += wpos.x * 0.2f;
            float new_cam_y = camAng.y + wpos.y * 0.2f;
            if(fabs(new_cam_y) < M_PI_2) 
                camAng.y = new_cam_y;

            camForward = {cos(camAng.x) * cos(camAng.y), 
                       sin(camAng.y), 
                       sin(camAng.x) * cos(camAng.y)};
        }

        void HandleKeys()
        {
            float moveSpeed = 0.3;

            // Slow move
            if(glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                moveSpeed *= .5;

            // Handle movement with WASD
            bool gW = glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS;
            bool gA = glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS;
            bool gS = glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS;
            bool gD = glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS;
            bool gQ = glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS;
            bool gE = glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS;
            if(gW | gA | gS | gD)
            {
                int dir = (gW | gD) ? 1 : -1; // Positive or negative dir?
                float ang_off_ = (gW | gS) ? 0 : M_PI_2; // Forward or to side?
                player_.m_move_vel = dir * moveSpeed * 
                        glm::vec3(cos(camAng.x + ang_off_), 0, sin(camAng.x + ang_off_));
            }
            else if(gQ | gE)
            {
                float updown = (gQ | gE) ? (gQ ? -1 : 1) : 0;
                player_.m_move_vel = moveSpeed * glm::vec3(0., updown, 0.);
            }
            else 
            {
                player_.m_move_vel = glm::vec3(0.0f);
            }

            float time = glfwGetTime();
            if(glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS && time - centerPrevTime > 0.5)
            {
                centerCam = !centerCam;
                centerPrevTime = time;
            }

            bool gLft = glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS;
            bool gRgt = glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS;
            if(gLft | gRgt)
            {
                float inc = gLft ? -1 : 1;
                myMeshAng.x += inc * .03;
            }

            // End game
            if(glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window_, 1);
            }
        }
};

template< typename T >
inline
std::vector< T > to_std_vector( const boost::python::object& iterable )
{
    return std::vector< T >( boost::python::stl_input_iterator< T >( iterable ),
                             boost::python::stl_input_iterator< T >( ) );
}

Graphics setup(const boost::python::list& myPts, const boost::python::list& myInds, const boost::python::list& myNormals,
        int prim_type)
{
    auto my_pts_vec = to_std_vector<float>(myPts);
    auto my_inds_vec = to_std_vector<unsigned int>(myInds);
    auto my_normals_vec = to_std_vector<float>(myNormals);
    return Graphics(my_pts_vec, my_inds_vec, my_normals_vec, prim_type);
}

void draw_triangles(const boost::python::list& myPts, const boost::python::list& myInds, const boost::python::list& myNormals)
{
    return setup(myPts, myInds, myNormals, 4).RunMainLoop();
}

void draw_lines(const boost::python::list& myPts, const boost::python::list& myColors)
{
    auto empty = boost::python::list();
    return setup(myPts, empty, myColors, 3).RunMainLoop();
}

void draw_points(const boost::python::list& myPts, const boost::python::list& myColors)
{
    auto empty = boost::python::list();
    return setup(myPts, empty, myColors, 0).RunMainLoop();
}

void animate(const boost::python::list& myPts, const boost::python::list& myColors, int tsteps, float delay, int lines)
{
    int N = boost::python::len(myPts) / 3; // 3 floats per coordinate.
    int inc = N / tsteps;
    auto empty = boost::python::list();
    Graphics gp = setup(myPts, empty, myColors, lines);
    int start = 0;
    float prev_time = 0.0;
    do
    {
        gp.myMesh.SetDrawSubset(start, inc);
        float dt = glfwGetTime() - prev_time;
        if(dt >= delay)
        {
            // Update animation.
            start = (start + inc) % N;
            prev_time = glfwGetTime();
        }
    }
    while(gp.Render());
}

BOOST_PYTHON_MODULE(hazelviz)
{
    using namespace boost::python;
    def("draw_triangles", draw_triangles);
    def("draw_lines", draw_lines);
    def("draw_points", draw_points);
    def("animate", animate);
}
