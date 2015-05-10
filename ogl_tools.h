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
	enum render_type { NORMAL, TEXT, ABSOLUTE };

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
		const float getAspectRatio() const { return aspect_ratio; }
		const glm::vec4 getBackgroundColor() const { return background_color; }

		int getWindowHeight() const { return window_height; }
		int getWindowWidth() const { return window_width; }

		GLint ogl_context::getShaderGLint(GLchar* name);

		void setBackgroundColor(glm::vec4 color) { glClearColor(color.x, color.y, color.z, color.w); background_color = color; }

	private:
		GLuint createShader(std::string file, GLenum type);
		GLuint createProgram(std::string vert_file, std::string frag_file);

		GLint element_color_ID;
		glm::vec4 background_color;
		GLFWwindow* window;
		bool errors = true;
		int window_height, window_width;
		std::string window_title;
		std::vector<std::string> display_errors;

		std::map<GLchar*, boost::shared_ptr<GLint> > glint_map;

		GLuint program_ID;

		float aspect_ratio;
	};

	//ogl_camera is a projection matrix manipulator, this is a base class, from which specific camera types should be derived
	//TODO make this class an abstract class
	class ogl_camera
	{
	public:
		ogl_camera(const boost::shared_ptr<key_handler> &kh, const boost::shared_ptr<ogl_context> &context, glm::vec3 position, glm::vec3 focus, float fov);
		~ogl_camera(){};

		void setViewMatrix(const glm::mat4 &vm) { view_matrix = vm; }
		const glm::mat4 getViewMatrix() const { return view_matrix; }
		const glm::mat4 getProjectionMatrix() const { return projection_matrix; }
		boost::shared_ptr<key_handler> getKeys() { return keys; }
		void setMVP(const boost::shared_ptr<ogl_context> &context, const glm::mat4 &model_matrix, const render_type &rt);

		const glm::vec3 getFocus() const { return camera_focus; }
		const glm::vec3 getPosition() const { return camera_position; }

		void setFocus(glm::vec3 focus) { camera_focus = focus; }
		void setPosition(glm::vec3 position) { camera_position = position; }

		virtual void updateCamera();

	private:
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		glm::mat4 previous_model_matrix;
		glm::mat4 aspect_scale_matrix;
		boost::shared_ptr<key_handler> keys;
		float aspect_scale;

		render_type current_render_type;

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
		ogl_camera_free(const boost::shared_ptr<key_handler> &kh, const boost::shared_ptr<ogl_context> &context, glm::vec3 position, float fov) :
			ogl_camera(kh, context, position, glm::vec3(position.x, position.y, position.z - 10.0f), fov)
		{
			strafe_distance = .1f;
			step_distance = .1f;
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

		virtual void updateCamera();
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

	class ogl_camera_iso : public ogl_camera
	{
	public:
		ogl_camera_iso(const boost::shared_ptr<key_handler> &kh, const boost::shared_ptr<ogl_context> &context, glm::vec3 position, float fov) :
			ogl_camera(kh, context, position, glm::vec3(position.x, position.y, position.z - 10.0f), fov)
		{
			strafe_distance = .1f;
			step_distance = .1f;
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

		~ogl_camera_iso(){};

		void printErrors();

		virtual void updateCamera();
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

		bool checkPress(int key, bool hold = true);
		bool checkMouse(int key, bool hold = true);
		glm::vec2 getCursorPosition() { 
			updateCursorPosition(); return glm::vec2(x_window_position, y_window_position); }

	private:
		void updateCursorPosition();
		double x_window_position;
		double y_window_position;
		std::map<int, bool> keys;
		std::map<int, bool> mouse;
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
		ogl_model(boost::shared_ptr<ogl_data> ogld) { opengl_data = ogld; }
		~ogl_model(){};

		virtual void draw(){};
		boost::shared_ptr<ogl_data> getOGLData() { return opengl_data; }
		glm::mat4 getModelMatrix() const { return model_matrix; }

	private:
		boost::shared_ptr<ogl_data> opengl_data;
		glm::mat4 model_matrix;
	};

	class ogl_model_static : public ogl_model
	{
	public: 
		ogl_model_static(boost::shared_ptr<ogl_data> ogld) : ogl_model(ogld) {};
		~ogl_model_static(){};

		virtual void draw(boost::shared_ptr<ogl_context> context, boost::shared_ptr<ogl_camera> camera);

	private:

	};

	class ogl_model_mobile : public ogl_model
	{
	public:
		ogl_model_mobile(boost::shared_ptr<ogl_data> ogld) : ogl_model(ogld) {};
		~ogl_model_mobile(){};

		virtual void draw(boost::shared_ptr<ogl_context> context, boost::shared_ptr<ogl_camera> camera);

	private:
		glm::mat4 model_matrix;
	};

	class ogl_model_animated : public ogl_model
	{
	public:
		ogl_model_animated(boost::shared_ptr<ogl_data> ogld) : ogl_model(ogld) {};
		~ogl_model_animated(){};

		virtual void draw(const boost::shared_ptr<ogl_context> &context, const boost::shared_ptr<ogl_camera> &camera);

	private:
		//first int is animation index, second is frame index, third is byte offset in opengl array
		std::map<int, std::map<int, int> > animation_indices;
		int current_animation;
		int current_frame;
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

	class static_text
	{
	public:
		static_text(const std::vector< std::pair<boost::shared_ptr<ogl_data>, glm::mat4> > &char_array,
			const glm::vec4 &color, const glm::vec4 &trans_color, bool transparent, const glm::vec2 &upper_left_position,
			const glm::vec2 &lower_right_position, float scale, float box_x = -1.0f, float box_y = -1.0f) : character_array(char_array)
		{
			text_color = color;
			transparency_color = trans_color;
			transparent_background = transparent;
			text_scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
			text_translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(upper_left_position.x, upper_left_position.y, 0.0f));
			upper_left = upper_left_position;
			lower_right = lower_right_position;
			x_bound = (box_x > 0.0f);
			y_bound = (box_y > 0.0f);
			box_width = box_x;
			box_height = box_y;
		};

		~static_text(){};

		void draw(const boost::shared_ptr<ogl_camera> &camera,
			const boost::shared_ptr<ogl_context> &context, GLchar* text_shader_ID, GLchar* text_color_shader_ID,
			GLchar* transparent_color_shader_ID);

		glm::vec2 getUpperLeft() const { return upper_left; }
		glm::vec2 getLowerRight() const;
		glm::vec2 getLowerLeft() const;
		glm::vec2 getUpperRight() const;

	private:
		glm::vec4 text_color;
		glm::vec4 transparency_color;
		glm::mat4 text_scale_matrix;
		bool transparent_background;
		glm::mat4 text_translation_matrix;

		glm::vec2 upper_left, lower_right;

		bool x_bound, y_bound;
		float box_width, box_height;
	
		std::vector< std::pair<boost::shared_ptr<ogl_data>, glm::mat4> > character_array;

		int line_count;
		int character_count;
	};

	class text_handler
	{
	public:
		text_handler(const boost::shared_ptr<ogl_context> &context, const char* text_image_path);
		~text_handler(){};

		boost::shared_ptr<static_text> getTextArray(std::string s, const boost::shared_ptr<ogl_context> &context, 
			bool italics, glm::vec4 color, glm::vec4 trans_color, bool transparent, glm::vec2 position, float scale, 
			float box_width = -1.0f, float box_height = -1.0f);

	private:
		map<int, boost::shared_ptr<ogl_data> > characters;
	};

	
}

#endif