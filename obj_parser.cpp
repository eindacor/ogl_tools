#include "ogl_tools.h"

namespace jep
{
	bool vertex_data::operator == (const vertex_data &other)
	{
		if (getUVOffset() != other.getUVOffset())
			return false;

		if (getNOffset() != other.getNOffset())
			return false;

		if (getStride() != other.getStride())
			return false;

		if (getVSize() != other.getVSize())
			return false;

		if (getVTSize() != other.getVTSize())
			return false;

		if (getVNSize() != other.getVNSize())
			return false;

		vector<float> other_v_data = other.getVData();
		for (int i = 0; i < v_data.size(); i++)
		{
			float difference = v_data.at(i) - other_v_data.at(i);
			if (abs(difference) > .000001f)
				return false;
		}

		vector<float> other_vt_data = other.getVTData();
		for (int i = 0; i < vt_data.size(); i++)
		{
			float difference = vt_data.at(i) - other_vt_data.at(i);
			if (abs(difference) > .000001f)
				return false;
		}

		vector<float> other_vn_data = other.getVNData();
		for (int i = 0; i < vn_data.size(); i++)
		{
			float difference = vn_data.at(i) - other_vn_data.at(i);
			if (abs(difference) > .000001f)
				return false;
		}

		return true;
	}

	void vertex_data::setVertexData()
	{
		all_data.insert(all_data.end(), v_data.begin(), v_data.end());
		all_data.insert(all_data.end(), vt_data.begin(), vt_data.end());
		all_data.insert(all_data.end(), vn_data.begin(), vn_data.end());

		if (v_data.size() > 0)
		{
			x = v_data.at(0);
			y = v_data.at(1);
			z = v_data.at(2);
			xy = glm::vec2(v_data.at(0), v_data.at(1));
			xyz = glm::vec3(v_data.at(0), v_data.at(1), v_data.at(2));

			if (v_data.size() > 3)
			{
				w = v_data.at(3);
				xyzw = glm::vec4(v_data.at(0), v_data.at(1), v_data.at(2), v_data.at(3));
			}

			else
			{
				w = 1.0f;
				xyzw = glm::vec4(v_data.at(0), v_data.at(1), v_data.at(2), 1.0f);
			}
		}

		else
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
			xy = glm::vec2(0.0f, 0.0f);
			xyz = glm::vec3(0.0f, 0.0f, 0.0f);
			w = 1.0f;
			xyzw = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		if (vt_data.size() > 0)
		{
			if (vt_data.size() == 1)
			{
				for (int i = 0; i < all_data.size(); i++)
				{
					cout << all_data[i] << endl;
				}
			}

			else {
				u = vt_data.at(0);
				v = vt_data.at(1);
				uv = glm::vec2(vt_data.at(0), vt_data.at(1));
			}
		}

		else
		{
			u = 0.0f;
			v = 0.0f;
			uv = glm::vec2(0.0f, 0.0f);
		}

		if (vn_data.size() > 0)
		{
			n_x = vn_data.at(0);
			n_y = vn_data.at(1);
			n_z = vn_data.at(2);
			n_xy = glm::vec2(vn_data.at(0), vn_data.at(1));
			n_xyz = glm::vec3(vn_data.at(0), vn_data.at(1), vn_data.at(2));
		}

		else
		{
			n_x = 0.0f;
			n_y = 0.0f;
			n_z = 0.0f;
			n_xy = glm::vec2(0.0f, 0.0f);
			n_xyz = glm::vec3(0.0f, 0.0f, 0.0f);
		}
	}

	void vertex_data::modifyPosition(const glm::mat4 &translation_matrix)
	{
		all_data.clear();

		v_data.clear();
		xyzw = translation_matrix * xyzw;
		v_data.push_back(xyzw.x);
		v_data.push_back(xyzw.y);
		v_data.push_back(xyzw.z);

		if (v_count > 3)
			v_data.push_back(xyzw.w);

		setVertexData();
	}

	void vertex_data::rotate(const glm::mat4 &rotation_matrix)
	{
		all_data.clear();

		v_data.clear();
		xyzw = rotation_matrix * xyzw;
		v_data.push_back(xyzw.x);
		v_data.push_back(xyzw.y);
		v_data.push_back(xyzw.z);

		if (v_count > 3)
			v_data.push_back(xyzw.w);

		if (vn_count > 0)
		{
			vn_data.clear();
			n_xyz = glm::vec3(rotation_matrix * glm::vec4(n_xyz, 1.0f));

			if (abs(n_xyz.x) < 0.0001f)
				n_xyz.x = 0.0f;

			if (abs(n_xyz.y) < 0.0001f)
				n_xyz.y = 0.0f;

			if (abs(n_xyz.z) < 0.0001f)
				n_xyz.z = 0.0f;

			//n_xyz = glm::normalize(n_xyz);
			vn_data.push_back(n_xyz.x);
			vn_data.push_back(n_xyz.y);
			vn_data.push_back(n_xyz.z);
		}

		setVertexData();
	}

	void mesh_data::addFace(const vector<vertex_data> &data)
	{
		faces.push_back(data);
		total_face_count++;
		vertex_count += data.size();

		vector<glm::vec3> tangent_bitangent = calcTangentBitangent(data);

		//add data to each respective all_data vector, for retrieving individual sets
		for (vector<vertex_data>::const_iterator it = data.begin(); it != data.end(); it++)
		{
			addVData(it->getVData());
			addVTData(it->getVTData());
			addVNData(it->getVNData());

			//adds tangent/bitangent once for each vertex
			addTangentBitangent(tangent_bitangent);
		}

		for (auto i : data)
		{
			bool match_found = false;

			//TODO use find methods instead of iterating through all vertices
			for (auto j : vertex_map)
			{
				if (i == j.second)
				{
					element_index.push_back(j.first);
					match_found = true;

					//average tangents found with new tangents
					tangent_map[j.first] += tangent_bitangent[0];
					bitangent_map[j.first] += tangent_bitangent[1];

					break;
				}
			}

			if (!match_found)
			{
				//add new index, vertex, tangent, and bitangent
				unsigned short new_index = vertex_map.size();
				element_index.push_back(new_index);

				std::pair<unsigned short, vertex_data> vertex_to_add(new_index, i);
				vertex_map.insert(vertex_to_add);

				std::pair<unsigned short, glm::vec3> tangent_to_add(new_index, tangent_bitangent[0]);
				tangent_map.insert(tangent_to_add);

				std::pair<unsigned short, glm::vec3> bitangent_to_add(new_index, tangent_bitangent[1]);
				bitangent_map.insert(bitangent_to_add);
			}
		}
	}

	void mesh_data::addTangentBitangent(const vector<glm::vec3> &tb)
	{
		for (int i = 0; i < 3; i++)
		{
			tangents.push_back(tb[0]);
			bitangents.push_back(tb[1]);
		}
	}

	const vector<float> mesh_data::getInterleaveData() const
	{
		vector<float> interleave_data;
		interleave_data.reserve(total_float_count);
		for (vector< vector<vertex_data> >::const_iterator faces_it = faces.begin();
		faces_it != faces.end(); faces_it++)
		{
			//object data format will be:
			//		position.x, position.y, position.z, [position.w],
			//		uv.x, [uv.y], [uv.w],
			//		normal.x, normal.y, normal.z,
			//	bracketed values are only included if they were in the original obj file

			//for each vertex in each face, pass the stored, ordered data to interleave_data
			for (vector<vertex_data>::const_iterator vertex_it = faces_it->begin();
			vertex_it != faces_it->end(); vertex_it++)
			{
				vector<float> all_face_data(vertex_it->getAllData());
				interleave_data.insert(interleave_data.end(), all_face_data.begin(), all_face_data.end());
			}
		}
		return interleave_data;
	}

	/*
	void mesh_data::getIndexedVertexData(const vector<unsigned short> &indices, vector<float> &v_data, vector<float> &vt_data, vector<float> &vn_data) const
	{
	vector<float> interleaved_vertices = getInterleaveData();
	v_data.clear();
	vt_data.clear();
	vn_data.clear();

	vector<vertex_data> all_vertices;

	for (auto i : faces)
	{
	all_vertices.push_back(i.at(0));
	all_vertices.push_back(i.at(1));
	all_vertices.push_back(i.at(2));
	}

	map<unsigned short, vertex_data > vertex_map;

	for (auto vertex : all_vertices)
	{
	bool match_found = false;
	for (auto i : vertex_map)
	{
	if (vertex == i.second)
	{
	match_found = true;
	indices.push_back(i.first);
	break;
	}
	}

	if (!match_found)
	{
	unsigned short new_index = vertex_map.size();
	indices.push_back(new_index);
	vector<float> v_data_to_add = vertex.getVData();
	vector<float> vt_data_to_add = vertex.getVTData();
	vector<float> vn_data_to_add = vertex.getVNData();
	v_data.insert(v_data.end(), v_data_to_add.begin(), v_data_to_add.end());
	vt_data.insert(vt_data.end(), vt_data_to_add.begin(), vt_data_to_add.end());
	vn_data.insert(vn_data.end(), vn_data_to_add.begin(), vn_data_to_add.end());

	vertex_map.insert(std::pair<unsigned short, vertex_data>(new_index, vertex));
	}
	}
	}
	*/

	const vector<float> mesh_data::getIndexedVertexData() const
	{
		vector<float> all_data;

		for (const auto vert_pair : vertex_map)
		{
			//includes vertex position data, uv data, and normal data
			vector<float> vertex_data_to_add = (vert_pair.second).getAllData();

			//append tangent data
			glm::vec3 tangent_data = tangent_map.at(vert_pair.first);
			vertex_data_to_add.push_back(tangent_data.x);
			vertex_data_to_add.push_back(tangent_data.y);
			vertex_data_to_add.push_back(tangent_data.z);

			//append bitangent data
			glm::vec3 bitangent_data = bitangent_map.at(vert_pair.first);
			vertex_data_to_add.push_back(bitangent_data.x);
			vertex_data_to_add.push_back(bitangent_data.y);
			vertex_data_to_add.push_back(bitangent_data.z);

			all_data.insert(all_data.end(), vertex_data_to_add.begin(), vertex_data_to_add.end());
		}

		return all_data;
	}

	const vector<float> mesh_data::getIndexedVertexData(vector<unsigned short> &indices) const
	{
		vector<float> unique_vertices;
		vector<float> interleaved_vertices = getInterleaveData();

		vector<vertex_data> all_vertices;

		for (auto i : faces)
		{
			all_vertices.push_back(i.at(0));
			all_vertices.push_back(i.at(1));
			all_vertices.push_back(i.at(2));
		}

		map<unsigned short, vertex_data > vertex_map;

		for (auto vertex : all_vertices)
		{
			bool match_found = false;
			for (auto i : vertex_map)
			{
				if (vertex == i.second)
				{
					match_found = true;
					indices.push_back(i.first);
					break;
				}
			}

			if (!match_found)
			{
				unsigned short new_index = vertex_map.size();
				indices.push_back(new_index);
				vector<float> data_to_add = vertex.getAllData();
				unique_vertices.insert(unique_vertices.end(), data_to_add.begin(), data_to_add.end());
				vertex_map.insert(std::pair<unsigned short, vertex_data>(new_index, vertex));
			}
		}

		return unique_vertices;
	}

	void mesh_data::modifyPosition(const glm::mat4 &translation_matrix)
	{
		all_v_data.clear();

		for (auto &face : faces)
		{
			for (auto &vertex : face)
			{
				vertex.modifyPosition(translation_matrix);
				vector<float> vertex_v_data = vertex.getVData();
				all_v_data.insert(all_v_data.end(), vertex_v_data.begin(), vertex_v_data.end());
			}
		}

		for (auto i : vertex_map)
			i.second.modifyPosition(translation_matrix);
	}

	void mesh_data::rotate(const glm::mat4 &rotation_matrix)
	{
		all_v_data.clear();
		all_vn_data.clear();

		for (auto &face : faces)
		{
			for (auto &vertex : face)
			{
				vertex.rotate(rotation_matrix);
				vector<float> vertex_v_data = vertex.getVData();
				vector<float> vertex_vn_data = vertex.getVNData();
				all_v_data.insert(all_v_data.end(), vertex_v_data.begin(), vertex_v_data.end());
				all_vn_data.insert(all_vn_data.end(), vertex_vn_data.begin(), vertex_vn_data.end());
			}
		}

		for (auto i : vertex_map)
			i.second.rotate(rotation_matrix);
	}

	vector< std::pair<glm::vec4, glm::vec4> > mesh_data::getMeshEdgesVec4() const
	{
		map< int, std::pair<glm::vec4, glm::vec4> > edges;
		int edge_counter = 0;

		//for each side of each triangle
		for (auto i : faces)
		{
			std::pair<glm::vec4, glm::vec4> edge1(i.at(0).xyzw, i.at(1).xyzw);
			std::pair<glm::vec4, glm::vec4> edge2(i.at(1).xyzw, i.at(2).xyzw);
			std::pair<glm::vec4, glm::vec4> edge3(i.at(2).xyzw, i.at(0).xyzw);

			edges.insert(std::pair<int, std::pair<glm::vec4, glm::vec4> >(edge_counter++, edge1));
			edges.insert(std::pair<int, std::pair<glm::vec4, glm::vec4> >(edge_counter++, edge2));
			edges.insert(std::pair<int, std::pair<glm::vec4, glm::vec4> >(edge_counter++, edge3));
		}

		vector<int> unique_edges;
		vector<int> shared_edges;

		for (auto i : edges)
		{
			//verity edge analyzed isn't already identified as shared
			if (std::find(shared_edges.begin(), shared_edges.end(), i.first) != shared_edges.end())
				continue;

			std::pair<glm::vec4, glm::vec4> i_edge = i.second;

			bool unique = true;

			for (auto j : edges)
			{
				//skip if the edge has already been confirmed as unique or shared
				if (std::find(shared_edges.begin(), shared_edges.end(), j.first) != shared_edges.end())
					continue;

				if (std::find(unique_edges.begin(), unique_edges.end(), j.first) != unique_edges.end())
					continue;

				std::pair<glm::vec4, glm::vec4> j_edge = j.second;

				//if each edge has similar points, in any order, add to shared edges
				if ((i_edge.second == j_edge.first && i_edge.second == j_edge.second) ||
					(i_edge.first == j_edge.second && i_edge.second == j_edge.first))
				{
					shared_edges.push_back(i.first);
					shared_edges.push_back(j.first);
					unique = false;
					break;
				}
			}

			if (unique)
				unique_edges.push_back(i.first);
		}

		vector< std::pair<glm::vec4, glm::vec4> > outer_edges;

		for (auto i : unique_edges)
			outer_edges.push_back(edges.at(i));

		return outer_edges;
	}

	vector< std::pair<glm::vec3, glm::vec3> > mesh_data::getMeshEdgesVec3() const
	{
		vector< std::pair<glm::vec3, glm::vec3> > vec3_data;
		vector< std::pair<glm::vec4, glm::vec4> > vec4_data = getMeshEdgesVec4();

		for (auto i : vec4_data)
		{
			std::pair<glm::vec3, glm::vec3> converted(glm::vec3(i.first), glm::vec3(i.second));
			vec3_data.push_back(converted);
		}

		return vec3_data;
	}

	vector< vector<glm::vec4> > mesh_data::getMeshTrianglesVec4() const
	{
		vector< vector<glm::vec4> > triangles;

		for (auto i : faces)
		{
			vector<glm::vec4> triangle = {
				i.at(0).xyzw,
				i.at(1).xyzw,
				i.at(2).xyzw
			};

			triangles.push_back(triangle);
		}

		return triangles;
	}

	vector< vector<glm::vec3> > mesh_data::getMeshTrianglesVec3() const
	{
		vector< vector<glm::vec3> > triangles;

		for (auto i : faces)
		{
			vector<glm::vec3> triangle = {
				i.at(0).xyz,
				i.at(1).xyz,
				i.at(2).xyz
			};

			triangles.push_back(triangle);
		}

		return triangles;
	}

	void mesh_data::setMeshData()
	{
		if (faces.begin() != faces.end())
		{
			interleave_stride = faces.begin()->begin()->getStride();
			interleave_vt_offset = faces.begin()->begin()->getUVOffset();
			interleave_vn_offset = faces.begin()->begin()->getNOffset();

			v_size = faces.begin()->begin()->getVSize();
			vt_size = faces.begin()->begin()->getVTSize();
			vn_size = faces.begin()->begin()->getVNSize();

			total_float_count = (v_size + vt_size + vn_size) * faces.size();
		}
	}

	vector<vertex_data> createFaceFromVertices(const vector<int> &indices, const vector<vertex_data> &vertices)
	{
		vector<vertex_data> returned_vertices;

		for (vector<int>::const_iterator i = indices.cbegin(); i != indices.cend(); i++)
		{
			try
			{
				returned_vertices.push_back(vertices.at(*i));
			}

			catch (const std::out_of_range &oor) {}
		}

		return returned_vertices;
	}

	void addDataToMesh(mesh_data &mesh, const vector<vertex_data> &vertices)
	{
		for (vector<vertex_data>::const_iterator it = vertices.cbegin(); it != vertices.cend(); it++)
		{
			mesh.addVData(it->getVData());
			mesh.addVTData(it->getVTData());
			mesh.addVNData(it->getVNData());
		}
	}

	obj_contents::obj_contents(const char* obj_file)
	{
		v_index_counter = 1;
		vt_index_counter = 1;
		vn_index_counter = 1;
		vp_index_counter = 1;

		std::fstream file;
		file.open(obj_file, std::ifstream::in);

		if (!file.is_open())
		{
			string error = "unable to open obj file: ";
			error += obj_file;
			std::cout << error << std::endl;
			error_log.push_back(error);
			file.close();
			return;
		}

		bool end_of_vertex_data = false;

		meshes.push_back(mesh_data());
		vector<mesh_data>::iterator current_mesh = meshes.begin();

		vector<DATA_TYPE> index_order;

		string current_material;

		while (!file.eof())
		{
			string line;
			std::getline(file, line, '\n');

			DATA_TYPE type = getDataType(line);

			if (type == UNDEFINED_DATA_TYPE)
				continue;

			//"g" prefix indicates the previous geometry data has ended
			else if (type == OBJ_G)
			{
				current_mesh->setMeshName(extractName(line));
				end_of_vertex_data = true;
				continue;
			}

			if (type == OBJ_USEMTL)
			{
				current_material = extractName(line);
				current_mesh->setMaterialName(current_material);
				continue;
			}

			if (type == OBJ_MTLLIB)
			{
				mtl_filename = extractName(line);
				continue;
			}

			//detects if a new geometry is starting, resets params and operates on new mesh
			if (type == OBJ_V && end_of_vertex_data)
			{
				meshes.push_back(mesh_data());
				current_mesh = meshes.end() - 1;
				current_mesh->setMaterialName(current_material);
				end_of_vertex_data = false;
				index_order.clear();
			}

			if (type == OBJ_V || type == OBJ_VT || type == OBJ_VN || type == OBJ_VP)
			{
				if (std::find(index_order.begin(), index_order.end(), type) == index_order.end())
					index_order.push_back(type);

				vector<float> floats(extractFloats(line));
				addRawData(floats, type);
			}

			else if (type == OBJ_F)
			{
				//face_data contains the index list for each line (each face)
				//	1/1/1  2/2/2  3/3/3
				vector< vector<int> > extracted_face_data(extractFaceSequence(line));

				//generate vertex data objects from sequences passed
				vector<vertex_data> extracted_vertices;
				for (int i = 0; i < extracted_face_data.size(); i++)
				{
					int v_index = 0;
					int vt_index = 0;
					int vn_index = 0;
					int vp_index = 0;
					vector<float> position_data;
					vector<float> uv_data;
					vector<float> normal_data;

					for (int n = 0; n < index_order.size(); n++)
					{
						if (v_index = extracted_face_data[i][n] == 0)
							continue;

						switch (index_order[n])
						{
						case OBJ_V:
							v_index = extracted_face_data[i][n];
							position_data = raw_v_data.at(v_index);
							break;
						case OBJ_VT:
							vt_index = extracted_face_data[i][n];
							uv_data = raw_vt_data.at(vt_index);
							break;
						case OBJ_VN:
							vn_index = extracted_face_data[i][n];
							normal_data = raw_vn_data.at(vn_index);
							break;
						case OBJ_VP:
							vp_index = extracted_face_data[i][n];
							break;
						default: throw;
						}
					}

					vertex_data vert(position_data, uv_data, normal_data);
					extracted_vertices.push_back(vert);
				}

				//while loop allows for obj file to contain faces with >3 vertices
				int first = 0;
				int second = 1;
				int third = 2;

				while (third < extracted_face_data.size())
				{
					vector<vertex_data> face = createFaceFromVertices({ first, second, third }, extracted_vertices);
					current_mesh->addFace(face);

					if (second != first + 1)
						addDataToMesh(*current_mesh, face);

					second++;
					third++;
				}
			}
		}
		file.close();

		for (vector<mesh_data>::iterator it = meshes.begin(); it != meshes.end(); it++)
			it->setMeshData();
	}
	vector<glm::vec3> mesh_data::calcTangentBitangent(const vector<vertex_data> &face_data)
	{
		vertex_data v_data_0 = face_data[0];
		vertex_data v_data_1 = face_data[1];
		vertex_data v_data_2 = face_data[2];

		glm::vec3 v0(v_data_0.getVData()[0], v_data_0.getVData()[1], v_data_0.getVData()[2]);
		glm::vec3 v1(v_data_1.getVData()[0], v_data_1.getVData()[1], v_data_1.getVData()[2]);
		glm::vec3 v2(v_data_2.getVData()[0], v_data_2.getVData()[1], v_data_2.getVData()[2]);

		vector<float> uv0_vtdata = v_data_0.getVTData();
		glm::vec2 uv0(uv0_vtdata.size() > 1 ? uv0_vtdata[0] : 0.0, uv0_vtdata.size() > 1 ? uv0_vtdata[1] : 0.0);

		vector<float> uv1_vtdata = v_data_1.getVTData();
		glm::vec2 uv1(uv1_vtdata.size() > 1 ? uv1_vtdata[0] : 1.0, uv1_vtdata.size() > 1 ? uv1_vtdata[1] : 1.0);

		vector<float> uv2_vtdata = v_data_2.getVTData();
		glm::vec2 uv2(uv2_vtdata.size() > 1 ? uv2_vtdata[0] : 1.0, uv2_vtdata.size() > 1 ? uv2_vtdata[1] : 0.0);

		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		return vector<glm::vec3> {tangent, bitangent};
	}

	void obj_contents::addRawData(const vector<float> &floats, DATA_TYPE dt)
	{
		switch (dt)
		{
		case OBJ_V:
			raw_v_data[v_index_counter] = floats;
			v_index_counter++;
			break;
		case OBJ_VT:
			raw_vt_data[vt_index_counter] = floats;
			vt_index_counter++;
			break;
		case OBJ_VN:
			raw_vn_data[vn_index_counter] = floats;
			vn_index_counter++;
			break;
		case OBJ_VP:
			raw_vp_data[vp_index_counter] = floats;
			vp_index_counter++;
			break;
		default: break;
		}
	}

	const vector<float> extractFloats(const string &s)
	{
		vector<float> separated;
		string current_string = "";
		for (int i = 0; i < s.size(); i++)
		{
			bool add_string = false;
			if (s[i] == ' ')
				add_string = true;

			else current_string += s[i];

			if (i == s.size() - 1 && current_string.size() > 0)
				add_string = true;

			if (add_string)
			{
				try
				{
					float toAdd = std::stof(current_string);
					separated.push_back(toAdd);
				}

				catch (const std::out_of_range &oor) {}
				catch (const std::invalid_argument &ia) {}

				current_string.clear();
			}
		}

		return separated;
	}

	const vector< vector<int> > extractFaceSequence(const string &s)
	{
		vector< vector<int> > index_list;

		vector<int> digits;
		vector<int> sequence;
		bool values_begin = false;

		for (int i = 0; i < s.size(); i++)
		{
			if (s[i] == '/')
			{
				if (digits.size() == 0)
				{
					sequence.push_back(0);
					continue;
				}

				float extracted = 0;

				for (int n = 0; n < digits.size(); n++)
				{
					int nth = digits.size() - 1 - n;
					float multiplier = pow(10.0f, float(nth));
					float toAdd = digits[n] * multiplier;
					extracted += toAdd;
				}

				sequence.push_back(int(extracted));

				//resets counters
				digits.clear();
			}

			else if (s[i] >= '0' && s[i] <= '9')
			{
				int char_int = '0';
				char_int = s[i] - char_int;
				digits.push_back(char_int);
			}

			//if space found or end of string
			if ((s[i] == ' ' && values_begin) || i == s.size() - 1)
			{
				float extracted = 0;

				for (int n = 0; n < digits.size(); n++)
				{
					int nth = digits.size() - 1 - n;
					float multiplier = pow(10.0f, float(nth));
					float toAdd = digits[n] * multiplier;
					extracted += toAdd;
				}

				sequence.push_back(int(extracted));
				index_list.push_back(sequence);

				//resets counters
				sequence.clear();
				digits.clear();
			}

			else if (s[i] == ' ' && !values_begin)
				values_begin = true;
		}

		return index_list;
	}

	const string extractName(const string &line)
	{
		string name;
		bool name_begin = false;
		for (int i = 0; i < line.size(); i++)
		{
			if (line[i] == ' ' && !name_begin)
			{
				name_begin = true;
				continue;
			}

			else if (name_begin)
				name += line[i];
		}

		return name;
	}

	const DATA_TYPE getDataType(const string &line)
	{
		string prefix;
		for (int i = 0; i < line.size(); i++)
		{
			if (line[i] != ' ')
				prefix += line[i];

			else break;
		}

		if (prefix == "mtllib")
			return OBJ_MTLLIB;

		if (prefix == "v")
			return OBJ_V;

		if (prefix == "vt")
			return OBJ_VT;

		if (prefix == "vn")
			return OBJ_VN;

		if (prefix == "vp")
			return OBJ_VP;

		if (prefix == "f")
			return OBJ_F;

		if (prefix == "g")
			return OBJ_G;

		if (prefix == "usemtl")
			return OBJ_USEMTL;

		if (prefix == "newmtl")
			return MTL_NEWMTL;

		if (prefix == "Ka")
			return MTL_KA;

		if (prefix == "Kd")
			return MTL_KD;

		if (prefix == "Ks")
			return MTL_KS;

		if (prefix == "Ns")
			return MTL_NS;

		if (prefix == "Tr" || prefix == "d" || prefix == "Tf")
			return MTL_D;

		if (prefix == "map_Ka")
			return MTL_MAP_KA;

		if (prefix == "map_Kd")
			return MTL_MAP_KD;

		if (prefix == "map_Ks")
			return MTL_MAP_KS;

		if (prefix == "map_Ns")
			return MTL_MAP_NS;

		if (prefix == "map_d")
			return MTL_MAP_D;

		if (prefix == "map_bump" || prefix == "bump")
			return MTL_MAP_BUMP;

		if (prefix == "disp")
			return MTL_MAP_DISP;

		if (prefix == "decal")
			return MTL_DECAL;

		return UNDEFINED_DATA_TYPE;
	}

	const vector<mesh_data> generateMeshes(const char* file_path)
	{
		obj_contents contents(file_path);
		return contents.getMeshes();
	}

	const map<string, boost::shared_ptr<material_data> > generateMaterials(const char* file_path, boost::shared_ptr<texture_handler> &textures, const boost::shared_ptr<ogl_context> &context)
	{
		mtl_contents contents(file_path, textures, context);
		return contents.getMaterials();
	}

	const vector<float> material_data::getData(DATA_TYPE dt) const
	{
		vector<float> default_values = { 0.0f, 0.0f, 0.0f, 0.0f };
		map<DATA_TYPE, vector<float> >::const_iterator it = data.find(dt);
		if (it == data.end())
			return default_values;

		else return it->second;
	}

	mtl_contents::mtl_contents(const char* mtl_file, boost::shared_ptr<texture_handler> &textures, const boost::shared_ptr<ogl_context> &context)
	{
		std::fstream file;
		file.open(mtl_file, std::ifstream::in);

		if (!file.is_open())
		{
			string error = "unable to open mtl file: ";
			error += mtl_file;
			std::cout << error << std::endl;
			error_log.push_back(error);
			return;
		}

		bool data_set = false;

		map<string, boost::shared_ptr<material_data> >::iterator current_material;

		while (!file.eof())
		{
			string line;
			std::getline(file, line, '\n');

			DATA_TYPE type = getDataType(line);

			if (type == UNDEFINED_DATA_TYPE)
				continue;

			if (type == MTL_NEWMTL)
			{
				string mtl_name = extractName(line);
				materials[mtl_name] = boost::shared_ptr<material_data>(new material_data(mtl_name, context, textures));
				current_material = materials.find(mtl_name);
			}

			if (type == MTL_KD || type == MTL_KA || type == MTL_D)
			{
				vector<float> floats(extractFloats(line));
				current_material->second->setData(type, floats);
			}

			if (type == MTL_MAP_KD) {
				string diffuse_filename = extractName(line);
				current_material->second->setTextureData("diffuse", diffuse_filename);
			}

			if (type == MTL_MAP_BUMP)
			{
				string bumpmap_string = extractName(line);
				size_t filename_delimiter = bumpmap_string.find(" -bm ");
				string bump_filename = bumpmap_string.substr(0, filename_delimiter);
				current_material->second->setTextureData("bump", bump_filename);
				string extracted_intensity = bumpmap_string.substr(filename_delimiter + 4);
				current_material->second->setBumpValue(std::stof(extracted_intensity, 0));
			}
		}

		file.close();
	}
}