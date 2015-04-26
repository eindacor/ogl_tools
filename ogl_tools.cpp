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
		int window_width, int window_height)
	{
		int width = window_width;
		int height = window_height;
		errors = false;
		window_title = title;
		projection_matrix = glm::perspective(45.0f, float(width) / float(height), .1f, 150.0f);

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

		program_ID = createProgram(vert_file, frag_file);
		geometry_color_ID = glGetUniformLocation(program_ID, "geometry_color");
		MVP_ID = glGetUniformLocation(program_ID, "MVP");
		texture_ID = glGetUniformLocation(program_ID, "myTextureSampler");

		//z-buffer functions, prevent close objects being clipped by far objects
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		//glEnable(GL_CULL_FACE);

		//create vertex buffer object, set clear color
		if (errors == false)
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

	ogl_camera::ogl_camera(boost::shared_ptr<key_handler> kh, glm::vec3 position, glm::vec3 focus)
	{
		camera_focus = focus;
		camera_position = position;

		keys = kh;
		view_matrix = glm::lookAt(
			glm::vec3(position.x, position.y, position.z),	//position of camera
			glm::vec3(focus.x, focus.y, focus.z),			//position of focal point
			glm::vec3(0, 1, 0));								//up axis
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

	void ogl_camera_free::update()
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
}