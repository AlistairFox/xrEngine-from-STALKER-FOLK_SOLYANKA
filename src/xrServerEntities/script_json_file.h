#pragma once
#include "../jsonxx/jsonxx.h"
#include <string> 

using namespace jsonxx;

class CObjectJsonEx : public Object
{
	string_path directory_path;

public:
	CObjectJsonEx() {};
	~CObjectJsonEx() {};

	typedef Object o;

	bool load(LPCSTR file_name, LPCSTR path);
	bool save(LPCSTR file_name, LPCSTR path);

 
	bool has_array(LPCSTR key);
	Array get_array(LPCSTR key);
	void set_array(LPCSTR key, Array value);
	
	bool has_object(LPCSTR key);
	CObjectJsonEx get_object(LPCSTR key);
	void set_object(LPCSTR key, CObjectJsonEx table);

	bool has_bool(LPCSTR value);
	bool has_number(LPCSTR value);
	bool has_string(LPCSTR value);

	double get_number(LPCSTR key);
	bool get_bool(LPCSTR key);
	LPCSTR get_string(LPCSTR key);

	void set_string(LPCSTR key, LPCSTR value);
	void set_number(LPCSTR key, double value);
	void set_bool(LPCSTR key, bool value);

	void set_shared_str(LPCSTR key, shared_str value)
	{
		o::import(key, (String) value.c_str());
	};
	
	void get_shared_str(LPCSTR key, shared_str& val)
	{
		val._set(o::get<String>(key).c_str());
	}
};

class CJsonFile
{
 	string_path directory_path;
	Object json_file;

public:
	//CJsonFile();
	//virtual 			~CJsonFile();
	
	bool has_boolean(Object table, LPCSTR key);
	bool has_number(Object table, LPCSTR key);
	bool has_string(Object table, LPCSTR key);
	bool has_object(Object table, LPCSTR key);
	bool has_array(Object table, LPCSTR key);

	
	bool get_boolean(Object table, LPCSTR key);
	LPCSTR get_string(Object table, LPCSTR table_name);
	double get_number(Object table, LPCSTR table_name);
	Object get_object(Object table, LPCSTR table_name);
	Array  get_array(Object table, LPCSTR table_name);
	
	Object set_boolean(Object table, LPCSTR key, BOOL value);
	Object set_string(Object table, LPCSTR key,  LPCSTR value);
	Object set_number(Object table, LPCSTR key, double value);
	
	Object set_object(Object table, LPCSTR key, Object new_table);
	Object set_array(Object table, LPCSTR key, Array new_table);


	Array set_object_array(Array table,  Object new_table);
	Object get_object_from_array(Array array, int key, Object new_table); 
	int array_size(Array array);

	void SaveJSON(LPCSTR file_name, LPCSTR path);
	void LoadJSON(LPCSTR file_name, LPCSTR path);
};
 


 

