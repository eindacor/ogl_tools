#include "ogl_tools.h"

namespace jep
{
	const float getLineAngle(glm::vec2 first, glm::vec2 second, bool right_handed)
	{
		if (right_handed)
		{
			first.x *= -1.0f;
			second.x *= -1.0f;
		}

		//assumes line is going from first to second
		//angle returned is counter-clockwise offset from horizontal axis to the right
		if (floatsAreEqual(first.x, second.x))
		{
			if (first.y < second.y)
				return 90.0f;

			else return 270.0f;
		}

		if (floatsAreEqual(first.y, second.y))
		{
			if (first.x < second.x)
				return 0.0f;

			else return 180.0f;
		}

		float tangent = ((second.y - first.y) / (second.x - first.x));
		float pi = 3.14159265;
		float angle = atan(abs(tangent)) * (180 / pi);

		if (tangent < 0)
		{
			if (first.x < second.x)
				return 360.0f - angle;

			else return 180.0f - angle;
		}

		else
		{
			if (first.x < second.x)
				return angle;

			else return 180.0f + angle;
		}
	}

	const glm::vec4 rotatePointAroundOrigin(const glm::vec4 &point, const glm::vec4 &origin, const float degrees, const glm::vec3 &axis)
	{
		glm::vec4 delta_vec = point - origin;
		delta_vec.w = 1.0f;

		glm::mat4 origin_offset = glm::translate(glm::mat4(1.0f), glm::vec3(origin.x, origin.y, origin.z));
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), degrees, axis);
		glm::vec4 new_point = rotation * delta_vec;
		return origin_offset * new_point;
	}

	void loadTexture(const char* imagepath, GLuint &textureID)
	{
		std::string filepath = imagepath;
		int size = filepath.size();
		std::string suffix;
		if (size >= 4)
		{
			suffix += filepath[size - 4];
			suffix += filepath[size - 3];
			suffix += filepath[size - 2];
			suffix += filepath[size - 1];
		}
		else std::cout << imagepath << " could not be opened" << std::endl;

		std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

		if (suffix == ".bmp")
			loadBMP(imagepath, textureID);

		if (suffix == ".tga")
			loadTGA(imagepath, textureID);
	}

	void loadBMP(const char* imagepath, GLuint &textureID)
	{
		//data read from the header of the BMP file
		unsigned char header[54];
		unsigned int data_pos;
		unsigned int width, height;
		unsigned int image_size; // width * height * 3
		//actual RGB data
		unsigned char* data;
		FILE * file = fopen(imagepath, "rb");
		if (!file)
		{
			std::cout << imagepath << " could not be opened" << std::endl;
			return;
		}
		//checks to verify that the header is a 54-byte header
		if (fread(header, 1, 54, file) != 54)
		{
			std::cout << imagepath << " is not a correct BMP file" << std::endl;
			return;
		}
		//checks to verify first two characters are "BM"
		if (header[0] != 'B' || header[1] != 'M')
		{
			std::cout << imagepath << " is not a correct BMP file" << std::endl;
			return;
		}
		//read ints from the byte array
		data_pos = *(int*)&(header[0x0A]);
		image_size = *(int*)&(header[0x22]);
		width = *(int*)&(header[0x12]);
		height = *(int*)&(header[0x16]);
		//if BMP was misformatted, guess missing info
		if (image_size == 0)
			image_size = width * height * 3;
		if (data_pos == 0)
			data_pos = 54;
		data = new unsigned char[image_size];
		//read actual data from the file into the buffer
		fread(data, 1, image_size, file);
		fclose(file);
		//create opengl texture
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		//give the image to opengl
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);
		delete[] data;
	}

	void loadTGA(const char* imagepath, GLuint &textureID)
	{
		//data read from the header of the BMP file
		unsigned char ID_length;
		unsigned char color_map_type;
		unsigned char image_type;
		unsigned short first_map_index;
		unsigned short color_map_length;
		char bits_per_pixel;
		unsigned int x_origin;
		unsigned int y_origin;
		unsigned int width;
		unsigned int height;
		char pixel_depth;
		char image_descriptor;

		unsigned int file_size;
		//actual RGB data

		unsigned char* data;

		FILE * file = fopen(imagepath, "rb");
		if (!file)
		{
			std::cout << imagepath << " could not be opened" << std::endl;
			return;
		}

		long lCurPos, lEndPos;
		lCurPos = ftell(file);
		fseek(file, 0, 2);
		lEndPos = ftell(file);
		fseek(file, lCurPos, 0);
		file_size = lEndPos;

		data = new unsigned char[file_size / sizeof(char)];

		//read actual data from the file into the buffer
		unsigned char header[18];
		fread(header, 1, 18, file);

		ID_length = *(char*)&(header[0]);
		color_map_type = *(char*)&(header[1]);
		image_type = *(char*)&(header[2]);
		first_map_index = *(unsigned short*)&(header[3]);
		color_map_length = *(unsigned short*)&(header[5]);
		bits_per_pixel = *(char*)&(header[7]);
		x_origin = *(unsigned short*)&(header[8]);
		y_origin = *(unsigned short*)&(header[10]);
		width = *(unsigned short*)&(header[12]);
		height = *(unsigned short*)&(header[14]);
		pixel_depth = *(char*)&(header[16]);
		image_descriptor = *(char*)&(header[17]);

		//advances fread buffer if an offset is indicated
		unsigned char* data_offset = new unsigned char[first_map_index];
		fread(data_offset, 1, first_map_index, file);

		unsigned int image_size = int(width) * int(height) * 3;

		unsigned char* color_map = new unsigned char[image_size];
		fread(color_map, 1, image_size, file);

		//create opengl texture
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		//give the image to opengl
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, color_map);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);

		delete[] data;
		delete[] data_offset;
		delete[] color_map;
		fclose(file);
	}

	ogl_context::ogl_context(std::string title, std::string vert_file, std::string frag_file,
		int width, int height)
	{
		window_width = width;
		window_height = height;
		aspect_ratio = (float)window_width / (float)window_height;
		errors = false;
		window_title = title;

		//initialize GLFW
		if (!glfwInit())
		{
			display_errors.push_back("glfw failed to initialize");
			errors = true;
		}

		//version control/create window
		else
		{
			glfwWindowHint(GLFW_SAMPLES, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			window = glfwCreateWindow(width, height, &window_title[0], NULL, NULL);
		}

		//test window
		if (errors == false && window == NULL)
		{
			display_errors.push_back("window returned NULL");
			errors = true;
		}

		//make context current/initialize glew
		if (errors == false)
		{
			glfwMakeContextCurrent(window);

			glewExperimental = true;
			if (glewInit() != GLEW_OK)
			{
				display_errors.push_back("glew failed to initialize");
				errors = true;
			}
		}

		//TODO make values of each ID variable
		program_ID = createProgram(vert_file, frag_file);	

		//z-buffer functions, prevent close objects being clipped by far objects
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glEnable(GL_CULL_FACE);

		//create vertex buffer object, set clear color
		if (errors == false)
		{
			background_color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			setBackgroundColor(background_color);
		}
	}

	ogl_context::~ogl_context()
	{
		//cleanup OpenGL/GLFW
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void ogl_context::printErrors()
	{
		for (std::vector<std::string>::const_iterator i = display_errors.begin(); i != display_errors.end(); i++)
			std::cout << *i << std::endl;
	}

	GLuint ogl_context::createShader(std::string file, GLenum type)
	{
		GLuint target_ID = glCreateShader(type);

		std::string code_string;
		//convert glsl file into a string
		std::ifstream shader_file;
		shader_file.open(file, std::ifstream::in);
		while (shader_file.good())
		{
			std::string line;
			std::getline(shader_file, line);
			code_string += line + '\n';
		}

		//create const char* from string of code
		const char* code_char = code_string.c_str();

		//compile shader
		glShaderSource(target_ID, 1, &code_char, NULL);
		glCompileShader(target_ID);

		//test compilation, return success/failure
		GLint status;
		glGetShaderiv(target_ID, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			std::string error;
			error += file;
			error += " failed to compile: ";

			GLint log_length;
			glGetShaderiv(target_ID, GL_INFO_LOG_LENGTH, &log_length);
			std::vector<char> info_log(log_length + 1);

			glGetShaderInfoLog(target_ID, log_length, NULL, &info_log[0]);

			for (std::vector<char>::const_iterator i = info_log.begin(); i != info_log.end(); i++)
				error += *i;

			std::cout << error << std::endl;

			return 0;
		}

		return target_ID;
	}

	GLuint ogl_context::createProgram(std::string vert_file, std::string frag_file)
	{
		//create program handle
		GLuint program_ID = glCreateProgram();
		GLuint fragment_shader_ID = createShader(frag_file, GL_FRAGMENT_SHADER);
		GLuint vertex_shader_ID = createShader(vert_file, GL_VERTEX_SHADER);

		//attach shaders, link program
		glAttachShader(program_ID, fragment_shader_ID);
		glAttachShader(program_ID, vertex_shader_ID);
		glLinkProgram(program_ID);

		//check link, return success/failure
		GLint status;
		glGetProgramiv(program_ID, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			std::string error = "program failed to link: ";

			GLint log_length;
			glGetProgramiv(program_ID, GL_INFO_LOG_LENGTH, &log_length);
			std::vector<char> info_log(log_length + 1);
			glGetProgramInfoLog(program_ID, log_length, NULL, &info_log[0]);

			for (std::vector<char>::iterator i = info_log.begin(); i != info_log.end(); i++)
				error += *i;

			std::cout << error << std::endl;
			return 0;
		}

		//detach shaders when complete
		glDetachShader(program_ID, fragment_shader_ID);
		glDetachShader(program_ID, vertex_shader_ID);

		return program_ID;
	}

	GLint ogl_context::getShaderGLint(GLchar* name)
	{
		if (glint_map.find(name) == glint_map.end())
		{
			boost::shared_ptr<GLint> GLintID(new GLint(glGetUniformLocation(program_ID, name)));
			glint_map.insert(std::pair<GLchar*, boost::shared_ptr<GLint> >(name, GLintID));		
		}

		return *(glint_map.at(name));
	}

	ogl_camera::ogl_camera(const boost::shared_ptr<key_handler> &kh, const boost::shared_ptr<ogl_context> &context, glm::vec3 position, glm::vec3 focus, float fov)
	{
		camera_focus = focus;
		camera_position = position;
		aspect_scale = (float)context->getWindowWidth() / (float)context->getWindowHeight();

		keys = kh;
		view_matrix = glm::lookAt(
			glm::vec3(position.x, position.y, position.z),	//position of camera
			glm::vec3(focus.x, focus.y, focus.z),			//position of focal point
			glm::vec3(0, 1, 0));								//up axis

		projection_matrix = glm::perspective(fov, aspect_scale, .1f, 150.0f);
		aspect_scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / context->getAspectRatio(), 1.0f, 1.0f));
	}

	void ogl_camera::updateCamera()
	{
		glm::mat4 view_matrix = glm::lookAt(
			glm::vec3(getPosition().x, getPosition().y, getPosition().z),	//position of camera
			glm::vec3(getFocus().x, getFocus().y, getFocus().z),		//position of focal point
			glm::vec3(0, 1, 0));

		setViewMatrix(view_matrix);
	};

	void ogl_camera::setMVP(const boost::shared_ptr<ogl_context> &context, const glm::mat4 &model_matrix, const render_type &rt)
	{
		if (current_render_type == rt && model_matrix == previous_model_matrix)
			return;

		current_render_type = rt;
		glm::mat4 MVP;

		switch(current_render_type)
		{
		case NORMAL:
			previous_model_matrix = model_matrix;
			glUniform1i(context->getShaderGLint("use_lighting"), true);
			MVP = projection_matrix * view_matrix * model_matrix;
			glUniformMatrix4fv(context->getShaderGLint("MVP"), 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(context->getShaderGLint("model_matrix"), 1, GL_FALSE, &model_matrix[0][0]);
			glUniformMatrix4fv(context->getShaderGLint("view_matrix"), 1, GL_FALSE, &view_matrix[0][0]);
			break;

		case TEXT: //aspect ratio is adjusted in within the code, since aspect ratio adjustments need to be made before the objects are translated
			previous_model_matrix = model_matrix;
			MVP = model_matrix;
			//MVP = model_matrix * aspect_scale_matrix;
			glUniform1i(context->getShaderGLint("use_lighting"), false);
			glUniformMatrix4fv(context->getShaderGLint("MVP"), 1, GL_FALSE, &MVP[0][0]);
			break;

		case ABSOLUTE:
			//aspect ratio is adjusted in within the code, since aspect ratio adjustments need to be made before the objects are translated
			previous_model_matrix = model_matrix;
			MVP = model_matrix;
			glUniform1i(context->getShaderGLint("use_lighting"), false);
			glUniformMatrix4fv(context->getShaderGLint("MVP"), 1, GL_FALSE, &MVP[0][0]);
			break;

		default:
			previous_model_matrix = model_matrix;
			MVP = projection_matrix * view_matrix * model_matrix;
			glUniformMatrix4fv(context->getShaderGLint("MVP"), 1, GL_FALSE, &MVP[0][0]);
			break;
		}
	}

	/*
	void ogl_camera_rigged::update()
	{
		if (target != NULL)
		{
			glm::vec3 origin = glm::vec3(target->getOrigin());
			glm::vec3 camera_focus = origin + focal_offset;
			glm::vec3 camera_position = origin + position_offset;

			glm::mat4 view_matrix = glm::lookAt(
				camera_position,				//position of camera
				camera_focus,					//position of focal point
				glm::vec3(0, 1, 0));

			setViewMatrix(view_matrix);
		}
	}
	*/

	void ogl_camera_free::stepCamera(float dist)
	{
		glm::vec4 camera_location = glm::vec4(getPosition(), 1.0f);
		glm::vec4 camera_focus = glm::vec4(getFocus(), 1.0f);

		glm::mat4 original_translate = glm::translate(
			glm::mat4(1.0f), glm::vec3(camera_location.x, camera_location.y, camera_location.z));
		glm::vec4 delta_vec(camera_focus - camera_location);
		float angle = getLineAngle(glm::vec2(camera_location.x, camera_location.z),
			glm::vec2(camera_focus.x, camera_focus.z), true);

		// must be negative distance because opengl uses right-handed coordinates
		glm::vec4 step_offset(-1.0f * dist, 0.0f, 0.0f, 1.0f);
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec4 new_location = rotation * step_offset;
		camera_location = original_translate * new_location;
		glm::mat4 focus_translation = glm::translate(glm::mat4(1.0f), glm::vec3(delta_vec.x, delta_vec.y, delta_vec.z));
		camera_focus = focus_translation * camera_location;

		setFocus(glm::vec3(camera_focus.x, camera_focus.y, camera_focus.z));
		setPosition(glm::vec3(camera_location.x, camera_location.y, camera_location.z));
	}

	void ogl_camera_free::strafeCamera(float dist)
	{
		glm::vec4 camera_location = glm::vec4(getPosition(), 1.0f);
		glm::vec4 camera_focus = glm::vec4(getFocus(), 1.0f);

		glm::mat4 original_translate = glm::translate(
			glm::mat4(1.0f), glm::vec3(camera_location.x, camera_location.y, camera_location.z));
		glm::vec4 delta_vec(camera_focus - camera_location);
		float angle = getLineAngle(glm::vec2(camera_location.x, camera_location.z),
			glm::vec2(camera_focus.x, camera_focus.z), true);
		angle -= 90.0f;

		// must be negative distance because opengl uses right-handed coordinates
		glm::vec4 step_offset(-1.0f * dist, 0.0f, 0.0f, 1.0f);
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec4 new_location = rotation * step_offset;
		camera_location = original_translate * new_location;
		glm::mat4 focus_translation = glm::translate(glm::mat4(1.0f), glm::vec3(delta_vec.x, delta_vec.y, delta_vec.z));
		camera_focus = focus_translation * camera_location;

		setFocus(glm::vec3(camera_focus.x, camera_focus.y, camera_focus.z));
		setPosition(glm::vec3(camera_location.x, camera_location.y, camera_location.z));
	}

	void ogl_camera_free::rotateCamera(float degrees)
	{
		glm::vec4 camera_location = glm::vec4(getPosition(), 1.0f);
		glm::vec4 camera_focus = glm::vec4(getFocus(), 1.0f);

		camera_focus = rotatePointAroundOrigin(
			camera_focus, camera_location, degrees, glm::vec3(0.0f, 1.0f, 0.0f));

		camera_rotation += degrees;

		if (camera_rotation > 360.f)
			camera_rotation -= 360.0f;

		if (camera_rotation < 0.0f)
			camera_rotation += 360.0f;

		setFocus(glm::vec3(camera_focus.x, camera_focus.y, camera_focus.z));
	}

	void ogl_camera_free::tiltCamera(float degrees)
	{
		glm::vec4 camera_location = glm::vec4(getPosition(), 1.0f);
		glm::vec4 camera_focus = glm::vec4(getFocus(), 1.0f);

		if (abs(camera_tilt + degrees) < 90.0f)
		{
			glm::vec4 axis_point = rotatePointAroundOrigin(
				camera_focus, camera_location, -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 axis(axis_point.x - camera_location.x, 0.0f, axis_point.z - camera_location.z);

			camera_focus = rotatePointAroundOrigin(
				camera_focus, camera_location, degrees, axis);

			camera_tilt += degrees;

			setFocus(glm::vec3(camera_focus.x, camera_focus.y, camera_focus.z));
		}
	}

	void ogl_camera_free::move(signed short n)
	{
		if (n == 0)
		{
			move_forward = false;
			move_backward = false;
			return;
		}

		else if (n < 0)
			move_forward = false;

		else if (n > 0)
			move_forward = true;

		move_backward = !move_forward;
	}

	void ogl_camera_free::rotate(signed short n)
	{
		if (n == 0)
		{
			rotate_left = false;
			rotate_right = false;
			return;
		}

		else if (n < 0)
			rotate_left = true;

		else if (n > 0)
			rotate_left = false;

		rotate_right = !rotate_left;
	}

	void ogl_camera_free::tilt(signed short n)
	{
		if (n == 0)
		{
			tilt_up = false;
			tilt_down = false;
			return;
		}

		else if (n < 0)
			tilt_up = false;

		else if (n > 0)
			tilt_up = true;

		tilt_down = !tilt_up;
	}

	void ogl_camera_free::strafe(signed short n)
	{
		if (n == 0)
		{
			strafe_left = false;
			strafe_right = false;
			return;
		}

		else if (n < 0)
			strafe_left = true;

		else if (n > 0)
			strafe_left = false;

		strafe_right = !strafe_left;
	}

	void ogl_camera_free::updateCamera()
	{
		if (getKeys()->checkPress(GLFW_KEY_A) || getKeys()->checkPress(GLFW_KEY_D))
		{
			if (getKeys()->checkPress(GLFW_KEY_D))
				strafe(1);

			else strafe(-1);
		}

		else strafe(0);

		if (getKeys()->checkPress(GLFW_KEY_S) || getKeys()->checkPress(GLFW_KEY_W))
		{
			if (getKeys()->checkPress(GLFW_KEY_S))
				move(-1);

			else move(1);
		}

		else move(0);

		if (getKeys()->checkPress(GLFW_KEY_UP) || getKeys()->checkPress(GLFW_KEY_DOWN))
		{
			if (getKeys()->checkPress(GLFW_KEY_UP))
				tilt(1);

			else tilt(-1);
		}

		else tilt(0);

		if (getKeys()->checkPress(GLFW_KEY_LEFT) || getKeys()->checkPress(GLFW_KEY_RIGHT))
		{
			if (getKeys()->checkPress(GLFW_KEY_LEFT))
				rotate(1);

			else rotate(-1);
		}

		else rotate(0);


		if (move_forward)
			stepCamera(step_distance);

		else if (move_backward)
			stepCamera(step_distance * -1.0f);

		if (rotate_left)
			rotateCamera(rotate_angle * -1.0f);

		else if (rotate_right)
			rotateCamera(rotate_angle);

		if (tilt_up)
			tiltCamera(tilt_angle);

		else if (tilt_down)
			tiltCamera(tilt_angle * -1.0f);

		if (strafe_left)
			strafeCamera(strafe_distance * -1.0f);

		else if (strafe_right)
			strafeCamera(strafe_distance);

		glm::mat4 view_matrix = glm::lookAt(
			glm::vec3(getPosition().x, getPosition().y, getPosition().z),	//position of camera
			glm::vec3(getFocus().x, getFocus().y, getFocus().z),		//position of focal point
			glm::vec3(0, 1, 0));

		setViewMatrix(view_matrix);
	}

	bool key_handler::checkPress(int key, bool include_held)
	{
		if (keys.find(key) == keys.end())
			keys[key] = GLFW_RELEASE;

		int state = glfwGetKey(context->getWindow(), key);

		bool first_press = (keys[key] == GLFW_RELEASE && state == GLFW_PRESS);
		bool was_held = (keys[key] == GLFW_PRESS && state == GLFW_PRESS);

		keys[key] = state;

		switch (include_held)
		{
		case true: return (state == GLFW_PRESS);
		case false: return (state == GLFW_PRESS && first_press);
		}
	}

	bool key_handler::checkMouse(int key, bool include_held)
	{
		if (mouse.find(key) == mouse.end())
			mouse[key] = GLFW_RELEASE;

		int state = glfwGetMouseButton(context->getWindow(), key);

		bool first_press = (mouse[key] == GLFW_RELEASE && state == GLFW_PRESS);
		bool was_held = (mouse[key] == GLFW_PRESS && state == GLFW_PRESS);

		mouse[key] = state;

		switch (include_held)
		{
		case true: return (state == GLFW_PRESS);
		case false: return (state == GLFW_PRESS && first_press);
		}
	}

	void key_handler::updateCursorPosition()
	{
		//glfw considers upper-left corner to be 0,0, and lower-right corner to be window width, window height
		//openGL considers upper-left to be -1.0, 1.0 and lower-right corner to be 1.0, -1.0
		double x_position;
		double y_position;
		glfwGetCursorPos(context->getWindow(), &x_position, &y_position);

		double window_height = (double)context->getWindowHeight();
		double window_width = (double)context->getWindowWidth();

		double center_y = window_height / 2.0f;
		double center_x = window_width / 2.0f;

		y_window_position = (center_y - y_position) / center_y;
		//reverse x dimension
		x_window_position = (center_x - x_position) / center_x * -1.0f;
	}

	//new geometry, new texture
	ogl_data::ogl_data(const boost::shared_ptr<ogl_context> &context,
					const char* texture_path, 
					GLenum draw_type, 
					const std::vector<float> &vec_vertices, 
					int position_vec_size, 
					int uv_vec_size, 
					int stride, 
					int uv_offset)
	{
		initializeGLuints();
		unique_texture = true;
		element_array_enabled = false;
		vertex_count = vec_vertices.size();

		glGenBuffers(1, VBO.get());
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, vec_vertices.size() * sizeof(float), &vec_vertices[0], draw_type);

		glGenVertexArrays(1, VAO.get());
		glBindVertexArray(*VAO);

		jep::loadTexture(texture_path, *TEX);
		//TODO make the name of the texture handler variable
		GLuint texture_ID = context->getShaderGLint("myTextureSampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *TEX);
		glUniform1i(texture_ID, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		//if points passed were vec3's, size = 3
		glVertexAttribPointer(0, position_vec_size, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glVertexAttribPointer(1, uv_vec_size, GL_FLOAT, GL_FALSE, stride, (void*)(uv_offset));

		glBindVertexArray(0);
	}

	//new geometry, existing texture
	ogl_data::ogl_data(const boost::shared_ptr<ogl_context> &context,
		const boost::shared_ptr<GLuint> &existing_texture,
		GLenum draw_type,
		const std::vector<float> &vec_vertices,
		int position_vec_size,
		int uv_vec_size,
		int stride,
		int offset)
	{
		initializeGLuints();
		TEX = existing_texture;
		unique_texture = false;
		element_array_enabled = false;
		vertex_count = vec_vertices.size();

		glGenBuffers(1, VBO.get());
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, vec_vertices.size() * sizeof(float), &vec_vertices[0], draw_type);

		glGenVertexArrays(1, VAO.get());
		glBindVertexArray(*VAO);

		GLuint texture_ID = context->getShaderGLint("myTextureSampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *TEX);
		glUniform1i(texture_ID, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		//if points passed were vec3's, size = 3
		glVertexAttribPointer(0, position_vec_size, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glVertexAttribPointer(1, uv_vec_size, GL_FLOAT, GL_FALSE, stride, (void*)(offset));

		glBindVertexArray(0);
	}

	//new geometry, indexed vertices, new texture
	ogl_data::ogl_data(const boost::shared_ptr<ogl_context> &context,
		const char* texture_path,
		GLenum draw_type,
		const std::vector<unsigned short> &indices,
		const std::vector<float> &vertex_data,
		int v_data_size,
		int vt_data_size,
		int vn_data_size)
	{
		index_count = indices.size();
		vertex_count = vertex_data.size();

		int uv_offset = v_data_size * sizeof(float);
		int normal_offset = uv_offset + (vt_data_size * sizeof(float));
		int stride = normal_offset + (vn_data_size * sizeof(float));

		initializeGLuints();
		unique_texture = true;
		element_array_enabled = true;
		glGenVertexArrays(1, VAO.get());
		glBindVertexArray(*VAO);

		glGenBuffers(1, VBO.get());
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), &vertex_data[0], draw_type);

		glGenBuffers(1, IND.get());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IND);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], draw_type);		

		jep::loadTexture(texture_path, *TEX);
		//TODO make the name of the texture handler variable
		GLuint texture_ID = context->getShaderGLint("myTextureSampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *TEX);
		glUniform1i(texture_ID, 0);
	
		//TODO revise so all data exists in one buffer
		//position
		//TODO pass size of each element to constructor instead of hard-coding
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, v_data_size, GL_FLOAT, GL_FALSE, stride, (void*)0);

		//uv
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, vt_data_size, GL_FLOAT, GL_FALSE, stride, (void*)(uv_offset));

		//normals
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, vn_data_size, GL_FLOAT, GL_FALSE, stride, (void*)(normal_offset));

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

	//new geometry, indexed vertices, existing texture
	ogl_data::ogl_data(const boost::shared_ptr<ogl_context> &context,
		const boost::shared_ptr<GLuint> &existing_texture,
		GLenum draw_type,
		const std::vector<unsigned short> &indices,
		const std::vector<float> &vertex_data,
		int v_data_size,
		int vt_data_size,
		int vn_data_size)
	{
		index_count = indices.size();
		vertex_count = vertex_data.size();

		int uv_offset = v_data_size * sizeof(float);
		int normal_offset = uv_offset + (vt_data_size * sizeof(float));
		int stride = normal_offset + (vn_data_size * sizeof(float));

		initializeGLuints();

		TEX = existing_texture;
		unique_texture = false;
		element_array_enabled = true;

		glGenVertexArrays(1, VAO.get());
		glBindVertexArray(*VAO);

		GLuint texture_ID = context->getShaderGLint("myTextureSampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *TEX);
		glUniform1i(texture_ID, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenBuffers(1, VBO.get());
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), &vertex_data[0], draw_type);

		glGenBuffers(1, IND.get());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IND);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], draw_type);

		//TODO revise so all data exists in one buffer
		//position
		//TODO pass size of each element to constructor instead of hard-coding
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, v_data_size, GL_FLOAT, GL_FALSE, stride, (void*)0);

		//uv
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, vt_data_size, GL_FLOAT, GL_FALSE, stride, (void*)(uv_offset));

		//normals
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, vn_data_size, GL_FLOAT, GL_FALSE, stride, (void*)(normal_offset));

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

	//TODO let texture handler delete all textures associated
	ogl_data::~ogl_data()
	{
		glDeleteVertexArrays(1, VAO.get());
		glDeleteBuffers(1, VBO.get());

		if (element_array_enabled)
			glDeleteBuffers(1, IND.get());

		if (unique_texture)
			glDeleteTextures(1, TEX.get());
	}

	void ogl_model_animated::draw(const boost::shared_ptr<ogl_context> &context, const boost::shared_ptr<ogl_camera> &camera)
	{
		int start_location_offset = animation_indices.at(current_animation).at(current_frame);
		//TODO find way to identify how many frames/vertices belong to each animation
		int frame_vertex_count = 0;
		glBindVertexArray(*(getOGLData()->getVAO()));
		glBindTexture(GL_TEXTURE_2D, *(getOGLData()->getTEX()));

		camera->setMVP(context, getModelMatrix(), NORMAL);
		glDrawArrays(GL_TRIANGLES, start_location_offset, frame_vertex_count);

		glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(0);
	}

	static_text::static_text(string s, text_justification tj, const boost::shared_ptr<text_handler> &text,
		const glm::vec4 &color, GLchar* text_enable_ID, GLchar* text_color_ID,
		const glm::vec2 &on_screen_position, float scale, float box_x, float box_y)
	{
		raw_text = s;
		text_shader_ID = text_enable_ID;
		text_color_shader_ID = text_color_ID;
		text_color = color;
		text_scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
		x_bound = (box_x > 0.0f);
		y_bound = (box_y > 0.0f);
		box_width = box_x;
		box_height = box_y;

		setPageData();

		//TODO add code for setting text array
	};

	void static_text::setPageData()
	{
		return;
	}

	//TODO for draw functions, allow passing of a map of shader ID's with their corresponding values
	//templatize if possible
	void static_text::draw(const boost::shared_ptr<ogl_camera> &camera,
		const boost::shared_ptr<ogl_context> &context)
	{
		//TODO try moving all of the "set" funcitons outside of the loop
		//enable text rendering in shader
		glUniform1i(context->getShaderGLint(text_shader_ID), true);

		//set text color
		glUniform4f(context->getShaderGLint(text_color_shader_ID),
			text_color.x, text_color.y, text_color.z, text_color.w);

		int counter = 0;
		for (const auto &i : character_array)
		{
			glBindVertexArray(*(i.first->getVAO()));
			glBindTexture(GL_TEXTURE_2D, *(i.first->getTEX()));
	
			//set mvp
			glm::mat4 character_translation_matrix = i.second;
			glm::mat4 model_matrix = text_translation_matrix * text_scale_matrix * character_translation_matrix;
			camera->setMVP(context, model_matrix, TEXT);

			//TODO change offset so it's variable depending on the character to be rendered
			//store all character data in the same buffer, offset accordingly based on current character
			glDrawArrays(GL_TRIANGLES, 0, i.first->getVertexCount());
		
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
		}

		//disable text rendering
		glUniform1i(context->getShaderGLint(text_shader_ID), false);
	}

	void static_text::draw(const boost::shared_ptr<ogl_camera> &camera,
		const boost::shared_ptr<ogl_context> &context, const glm::mat4 &position_matrix_override)
	{
		//TODO try moving all of the "set" funcitons outside of the loop
		//enable text rendering in shader
		glUniform1i(context->getShaderGLint(text_shader_ID), true);

		//set text color
		glUniform4f(context->getShaderGLint(text_color_shader_ID),
			text_color.x, text_color.y, text_color.z, text_color.w);

		int counter = 0;
		for (const auto &i : character_array)
		{
			glBindVertexArray(*(i.first->getVAO()));
			glBindTexture(GL_TEXTURE_2D, *(i.first->getTEX()));

			//set mvp
			glm::mat4 character_translation_matrix = i.second;
			glm::mat4 model_matrix = position_matrix_override * text_scale_matrix * character_translation_matrix;
			camera->setMVP(context, model_matrix, TEXT);

			//TODO change offset so it's variable depending on the character to be rendered
			//store all character data in the same buffer, offset accordingly based on current character
			glDrawArrays(GL_TRIANGLES, 0, i.first->getVertexCount());

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
		}

		//disable text rendering
		glUniform1i(context->getShaderGLint(text_shader_ID), false);
	}

	glm::vec2 static_text::getLowerRight() const
	{
		float x_position(x_bound ? upper_left.x + box_width : lower_right.x);
		float y_position(y_bound ? upper_left.y - box_height : lower_right.y);
		return glm::vec2(x_position, y_position);
	}

	glm::vec2 static_text::getLowerLeft() const
	{
		float x_position(upper_left.x);
		float y_position(y_bound ? upper_left.y - box_height : lower_right.y);
		return glm::vec2(x_position, y_position);
	}

	glm::vec2 static_text::getUpperRight() const
	{
		float x_position(x_bound ? upper_left.x + box_width : lower_right.x);
		float y_position(upper_left.y);
		return glm::vec2(x_position, y_position);
	}

	text_character::text_character(char character, const boost::shared_ptr<text_handler> &text, const glm::vec2 &anchor_point,
		text_justification tj, const glm::vec2 &screen_dimensions, bool italics)
	{
		c = character;
		dimensions = screen_dimensions;
		position = anchor_point;
		justification = tj;

		VAO = text->getOGLData()->getVAO();
		VBO = text->getOGLData()->getVBO();
		IND = text->getOGLData()->getIND();
		TEX = text->getOGLData()->getTEX();

		grid_index = 0;

		if (c == ' ')
			grid_index = 0;

		else grid_index = (int)c - 32;

		if (italics)
			grid_index += 96;

		setPositionMatrix();
	}

	void text_character::setPositionMatrix()
	{
		float step = 1.0f / 16.0f;
		float actual_x = (float)(grid_index % 16) * step + (0.5f * step);
		float actual_y = (float)(grid_index / 16) * step + (0.5f * step);
		glm::vec2 actual_centerpoint(actual_x, actual_y);

		float x_scale = dimensions.x / step;
		float y_scale = dimensions.y / step;

		glm::mat4 initial_translation = glm::translate(glm::mat4(1.0f), glm::vec3(actual_centerpoint * -1.0f, 0.0f));
		glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(x_scale, y_scale, 1.0f));
		glm::mat4 placement_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));

		glm::mat4 justification_matrix;

		switch (justification)
		{
		case UR:
			lower_left = position - dimensions;
			upper_left = position + glm::vec2(dimensions.x * -1.0f, 0.0f);
			upper_right = position;
			lower_right = position + glm::vec2(0.0f, dimensions.y * -1.0f);
			justification_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(dimensions.x * -0.5f, dimensions.y * -0.5f, 0.0f));
			break;
		case LR:
			lower_left = position + glm::vec2(dimensions.x * -1.0f, 0.0f);
			upper_left = position + glm::vec2(dimensions.x * -1.0f, dimensions.y * 1.0f);
			upper_right = position + glm::vec2(0.0f, dimensions.y * 1.0f);
			lower_right = position;
			justification_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(dimensions.x * -0.5f, dimensions.y * 0.5f, 0.0f));
			break;
		case LL:
			lower_left = position;
			upper_left = position + glm::vec2(0.0f, dimensions.y * 1.0f);
			upper_right = position + dimensions;
			lower_right = position + glm::vec2(dimensions.x * 1.0f, 0.0f);
			justification_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(dimensions.x * 0.5f, dimensions.y * 0.5f, 0.0f));
			break;
		case UL:
			lower_left = position + glm::vec2(0.0f, dimensions.y * -1.0f);
			upper_left = position;
			upper_right = position + glm::vec2(dimensions.x * 1.0f, 0.0f);
			lower_right = position + glm::vec2(dimensions.x * 1.0f, dimensions.y * -1.0f);
			justification_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(dimensions.x * 0.5f, dimensions.y * -0.5f, 0.0f));
			break;

		default:
			justification_matrix = glm::mat4(0.0f);
			break;
		}

		position_matrix = justification_matrix * placement_matrix * scale_matrix * initial_translation;
	}

	void text_character::draw(const boost::shared_ptr<ogl_context> &context, const boost::shared_ptr<ogl_camera> &camera)
	{
		glBindVertexArray(*(VAO));
		glBindTexture(GL_TEXTURE_2D, *(TEX));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(IND));

		camera->setMVP(context, position_matrix, TEXT);

		int offset = grid_index * 6 * sizeof(unsigned short);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)offset);

		glBindTexture(GL_TEXTURE_2D, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	text_handler::text_handler(const boost::shared_ptr<ogl_context> &context,
		const boost::shared_ptr<GLuint> &TEX, GLchar* transparent_color_shader_ID, glm::vec4 transparency_color)
	{
		//image file must be made as a 16 x 16 grid
		float step = 1.0f / 16.0f;

		default_TEX = TEX;

		//set transparent color
		glUniform4f(context->getShaderGLint(transparent_color_shader_ID),
			transparency_color.x, transparency_color.y, transparency_color.z, transparency_color.w);

		vector<float> vec_vertices;
		vec_vertices.reserve(289 * 8);

		for (int i = 0; i < 289; i++)
		{
			vector<float> vertex = {
				(i % 17) * step, (i / 17) * step, 0.0f,				//x, y, z
				(i % 17) * step, (i / 17) * step,					//u, v
				0.0f, 0.0f, 1.0f									//normal
			};

			vec_vertices.insert(vec_vertices.end(), vertex.begin(), vertex.end());
		}

		vector<unsigned short> indices;
		indices.reserve(271 * 6);

		for (int i = 0; i < 271; i++)
		{
			if ((i + 1) % 17 != 0)
			{
				indices.push_back(unsigned short(i));
				indices.push_back(unsigned short(i + 17));
				indices.push_back(unsigned short(i + 18));
				indices.push_back(unsigned short(i));
				indices.push_back(unsigned short(i + 18));
				indices.push_back(unsigned short(i + 1));
			}
		}

		opengl_data = boost::shared_ptr<ogl_data>(new ogl_data(context, default_TEX, GL_STATIC_DRAW, indices, vec_vertices, 3, 2, 3));
	}

	text_handler::~text_handler()
	{ 
		glDeleteTextures(1, default_TEX.get()); 

		for (auto &tex : font_map)
			glDeleteTextures(1, tex.second.get());
	}

	void text_handler::addFont(string font_name, const char* text_image_path)
	{
		if (font_map.find(font_name) == font_map.end())
			font_map[font_name] = boost::shared_ptr<GLuint>(new GLuint);

		jep::loadTexture(text_image_path, *font_map.at(font_name));
	}

	void text_handler::switchFont(string font_name)
	{
		if (font_map.find(font_name) != font_map.end())
		{
			opengl_data->overrideTEX(font_map.at(font_name));

			//for (auto &character : characters)
				//character.second->overrideTEX(font_map.at(font_name));
		}
	}

	line::line(glm::vec4 first, glm::vec4 second, glm::vec4 c)
	{
		p1 = first;
		p2 = second;
		color = c;

		VAO = boost::shared_ptr<GLuint>(new GLuint);
		VBO = boost::shared_ptr<GLuint>(new GLuint);

		glGenVertexArrays(1, VAO.get());
		glBindVertexArray(*VAO);
		glGenBuffers(1, VBO.get());
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);

		vector<float> vec_vertices{ first.x, first.y, first.z, second.x, second.y, second.z };

		glBufferData(GL_ARRAY_BUFFER, vec_vertices.size() * sizeof(float), &vec_vertices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDisableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	line::~line()
	{
		glDeleteVertexArrays(1, VAO.get());
		glDeleteBuffers(1, VBO.get());
	}

	void line::draw(const boost::shared_ptr<ogl_context> &context, const boost::shared_ptr<ogl_camera> &camera, bool absolute) const
	{
		glBindVertexArray(*VAO);
		glUniform1i(context->getShaderGLint("absolute_position"), absolute);

		glUniform1i(context->getShaderGLint("color_override"), true);
		glUniform4f(context->getShaderGLint("override_color"), color.x, color.y, color.z, color.w);

		camera->setMVP(context, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), 
			(absolute ? ABSOLUTE : NORMAL));

		glEnableVertexAttribArray(0);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableVertexAttribArray(0);

		glUniform1i(context->getShaderGLint("color_override"), false);
		glUniform1i(context->getShaderGLint("absolute_position"), false);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	rectangle::rectangle(glm::vec2 centerpoint, glm::vec2 dimensions, glm::vec4 c)
	{
		float half_width = dimensions.x / 2.0f;
		float half_height = dimensions.y / 2.0f;

		glm::vec2 upper_left(centerpoint.x - half_width, centerpoint.y + half_height);
		glm::vec2 upper_right(centerpoint.x + half_width, centerpoint.y + half_height);
		glm::vec2 lower_left(centerpoint.x - half_width, centerpoint.y - half_height);
		glm::vec2 lower_right(centerpoint.x + half_width, centerpoint.y - half_height);

		vec_vertices = vector < float > {
			lower_left.x, lower_left.y, 0.0f,
				upper_left.x, upper_left.y, 0.0f,
				upper_right.x, upper_right.y, 0.0f,
				lower_left.x, lower_left.y, 0.0f,
				upper_right.x, upper_right.y, 0.0f,
				lower_right.x, lower_right.y, 0.0f
		};

		color = c;

		VAO = boost::shared_ptr<GLuint>(new GLuint);
		VBO = boost::shared_ptr<GLuint>(new GLuint);

		glGenVertexArrays(1, VAO.get());
		glBindVertexArray(*VAO);
		glGenBuffers(1, VBO.get());
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);

		glBufferData(GL_ARRAY_BUFFER, vec_vertices.size() * sizeof(float), &vec_vertices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	rectangle::~rectangle()
	{
		glDeleteVertexArrays(1, VAO.get());
		glDeleteBuffers(1, VBO.get());
	}

	void rectangle::draw(const boost::shared_ptr<ogl_context> &context, const boost::shared_ptr<ogl_camera> &camera, bool absolute) const
	{
		glBindVertexArray(*VAO);
		glUniform1i(context->getShaderGLint("absolute_position"), absolute);

		glUniform1i(context->getShaderGLint("color_override"), true);
		glUniform4f(context->getShaderGLint("override_color"), color.x, color.y, color.z, color.w);

		camera->setMVP(context, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), 
			(absolute ? ABSOLUTE : NORMAL));

		glEnableVertexAttribArray(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);

		glUniform1i(context->getShaderGLint("color_override"), false);
		glUniform1i(context->getShaderGLint("absolute_position"), false);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void rectangle::draw(const boost::shared_ptr<ogl_context> &context, const boost::shared_ptr<ogl_camera> &camera,
		const glm::mat4 &model_matrix, bool absolute) const
	{
		glBindVertexArray(*VAO);
		glUniform1i(context->getShaderGLint("absolute_position"), absolute);

		glUniform1i(context->getShaderGLint("color_override"), true);
		glUniform4f(context->getShaderGLint("override_color"), color.x, color.y, color.z, color.w);

		camera->setMVP(context, model_matrix, (absolute ? (render_type)2 : (render_type)0));

		glEnableVertexAttribArray(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);

		glUniform1i(context->getShaderGLint("color_override"), false);
		glUniform1i(context->getShaderGLint("absolute_position"), false);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

