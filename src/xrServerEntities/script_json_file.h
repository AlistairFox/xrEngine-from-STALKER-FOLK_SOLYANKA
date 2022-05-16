#pragma once
#include "../jsonxx/jsonxx.h"

using namespace jsonxx;

class CJsonFile
{
 	string_path directory_path;

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

	void SaveJSON(Object table, LPCSTR file_name, LPCSTR path);
	Object LoadJSON(Object table, LPCSTR file_name, LPCSTR path);
};
 


 

