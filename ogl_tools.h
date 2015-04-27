#ifndef OPENGL_TOOLS_H
#define OPENGL_TOOLS_H

#include <glew.h>
#include <glfw3.h>
#include <string>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <boost/smart_ptr/shared_ptr.hpp>
#include "jep_util.h"

namespace jep
{
	class ogl_context;
	class key_handler;
	class ogl_camera;

	const float getLineAngle(glm::vec2 first, glm::vec2 second, bool right_handed);
	const glm::vec4 rotatePointAroundOrigin(const glm::vec4 &point, const glm::vec4 &origin, const float degrees, const glm::vec3 &axis);
	void loadTexture(const char* imagepath, GLuint &textureID);
	void loadBMP(const char* imagepath, GLuint &textureID);
	void loadTGA(const char* imagepath, GLuint &textureID);

	//ogl_context initializes glew, creates a glfw window, generates programs using shaders provided, 
	//and stores program and texture GLuints to be used by other objects
	class ogl_context
	{
	public:
		ogl_context(std::string title, std::string vert_file, std::string frag_file,
			int window_width, int window_height);
		~ogl_context();

		void printErrors();

		GLFWwindow* getWindow() { return window; }
		bool getErrors() { return errors; }

		void clearBuffers() const {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glUseProgram(program_ID);
		}
		void swapBuffers() const { glfwSwapBuffers(window); }

		const GLuint getProgramID() const { return program_ID; }
		const GLuint getTextureID() const { return program_ID; }
		const GLint getColorID() const { return geometry_color_ID; }
		const GLint getMVPID() const { return MVP_ID; }
		const glm::mat4 getProjectionMatrix() const { return projection_matrix; }

	private:
		GLuint createShader(std::string file, GLenum type);
		GLuint createProgram(std::string vert_file, std::string frag_file);

		GLint element_color_ID;
		glm::vec4 background_color;
		GLFWwindow* window;
		bool errors = true;
		std::string window_title;
		std::vector<std::string> display_errors;

		glm::mat4 projection_matrix;

		GLuint program_ID;
		GLuint texture_ID;
		GLint geometry_color_ID;
		GLint MVP_ID;
	};

	//ogl_camera is a projection matrix manipulator, this is a base class, from which specific camera types should be derived
	//TODO make this class an abstract class
	class ogl_camera
	{
	public:
		ogl_camera(boost::shared_ptr<key_handler> kh, glm::vec3 position, glm::vec3 focus);
		~ogl_camera(){};

		void setViewMatrix(const glm::mat4 &vm) { view_matrix = vm; }
		const glm::mat4 getViewMatrix() const { return view_matrix; }
		boost::shared_ptr<key_handler> getKeys() { return keys; }

		const glm::vec3 getFocus() const { return camera_focus; }
		const glm::vec3 getPosition() const { return camera_position; }

		void setFocus(glm::vec3 focus) { camera_focus = focus; }
		void setPosition(glm::vec3 position) { camera_position = position; }

		virtual void updateCamera() {};

	private:
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		boost::shared_ptr<key_handler> keys;

		glm::vec3 camera_focus;
		glm::vec3 camera_position;
	};

	/*
	class ogl_camera_rigged : public ogl_camera
	{
	public:
		ogl_camera_rigged(glm::vec3 p_offset, glm::vec3 f_offset) :
			ogl_camera(NULL, p_offset, f_offset),
			position_offset(p_offset),
			focal_offset(f_offset) {
			update();
		}
		~ogl_camera_rigged(){};

		void setTarget(boost::shared_ptr<model> t) { target = t; updateCamera(); }

		void update();

	private:
		glm::vec3 position_offset;
		glm::vec3 focal_offset;
		boost::shared_ptr<model> target;
	};
	*/

	//ogl_camera_free is a specific derived class of ogl_camera that allows keyboard inputs to modify the position
	//TODO enable customization of stepping distances
	class ogl_camera_free : public ogl_camera
	{
	public:
		ogl_camera_free(boost::shared_ptr<key_handler> kh, glm::vec3 position) :
			ogl_camera(kh, position, glm::vec3(position.x, position.y, position.z - 10.0f))
		{
			strafe_distance = .3f;
			step_distance = .3f;
			tilt_angle = 2.0f;
			rotate_angle = 2.0f;

			camera_tilt = 0.0f;
			camera_rotation = 0.0f;

			move_forward = false;
			move_backward = false;
			rotate_left = false;
			rotate_right = false;
			tilt_up = false;
			tilt_down = false;
			strafe_left = false;
			strafe_right = false;
		}

		~ogl_camera_free(){};

		void printErrors();

		virtual void update();
		void setPrintMovement(bool b) { print_movement = b; }

		void stepCamera(float dist);
		void strafeCamera(float dist);
		void rotateCamera(float degrees);
		void tiltCamera(float degrees);

		float camera_tilt;
		float camera_rotation;

		void move(signed short n);
		void rotate(signed short n);
		void tilt(signed short n);
		void strafe(signed short n);

	private:
		float strafe_distance;
		float step_distance;
		float tilt_angle;
		float rotate_angle;

		bool move_forward;
		bool move_backward;
		bool rotate_left;
		bool rotate_right;
		bool tilt_up;
		bool tilt_down;
		bool strafe_left;
		bool strafe_right;
		bool print_movement;
	};

	//key_handler interprets keystrokes using the ogl_context provided
	class key_handler
	{
	public:
		key_handler(boost::shared_ptr<ogl_context> ch) { context = ch; }
		~key_handler(){};

		bool checkPress(int key);

	private:
		std::map<int, bool> keys;
		boost::shared_ptr<ogl_context> context;
	};

	//class that handles VBO/VAO data for meshes that share a texture map
	class ogl_data
	{
	public:
		ogl_data(boost::shared_ptr<ogl_context> context,
				const char* texture_path,
				GLenum draw_type, 
				std::vector<float> vec_vertices, 
				int position_vec_size, 
				int uv_vec_size, 
				int stride, 
				int offset);
		~ogl_data();

		const int getVertexCount() const { return vertex_count; }

		boost::shared_ptr<GLuint> getVBO() const { return VBO; }
		boost::shared_ptr<GLuint> getVAO() const { return VAO; }
		boost::shared_ptr<GLuint> getTEX() const { return TEX; }

	private:
		boost::shared_ptr<GLuint> VBO;
		boost::shared_ptr<GLuint> VAO;
		boost::shared_ptr<GLuint> TEX;
		int vertex_count;
	};

	//class that stores/renders multiple ogl_data objects
	class ogl_model
	{
	public:
		ogl_model();
		~ogl_model(){};
	};

	class ogl_scene
	{
	public:
		ogl_scene(boost::shared_ptr<ogl_camera> cam, boost::shared_ptr<ogl_context> con);
		~ogl_scene(){};



	private:
		boost::shared_ptr<ogl_camera> camera;
		boost::shared_ptr<ogl_context> context;
		
	};
}

#endif