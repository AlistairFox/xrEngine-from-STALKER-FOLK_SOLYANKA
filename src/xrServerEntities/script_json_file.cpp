#include "stdafx.h"
#include "script_json_file.h"	


#include <fstream>
#include <iostream>
#include <fstream>
 
bool CJsonFile::has_boolean(Object table, LPCSTR key)
{
	return table.has<Boolean>(key);
}

bool CJsonFile::has_number(Object table, LPCSTR key)
{
	return table.has<Number>(key);
}

bool CJsonFile::has_string(Object table, LPCSTR key)
{
	return table.has<String>(key);
}

bool CJsonFile::has_object(Object table, LPCSTR key)
{
	return table.has<Object>(key);
}

bool CJsonFile::has_array(Object table, LPCSTR key)
{
	return table.has<Array>(key);
}

bool CJsonFile::get_boolean(Object table, LPCSTR key)
{
	if (table.has<Boolean>(key))
		return table.get<Boolean>(key);
	else
		return false;
}

double CJsonFile::get_number(Object table, LPCSTR key)
{
	if (table.has<Number>(key))
		return table.get<Number>(key);
 

	return 0;
}

LPCSTR CJsonFile::get_string(Object table, LPCSTR key)
{
	if (table.has<String>(key))
	{
		string256 name;

		xr_strcpy(name, table.get<String>(key).c_str());
		
		return name;
	}
	else
		return "nil";
}

Object CJsonFile::get_object(Object table, LPCSTR key)
{				
	if (table.has<Object>(key))
		return table.get<Object>(key);
 
	Object newtt;
	return newtt;
	 
}

Array CJsonFile::get_array(Object table, LPCSTR key)
{
	if (table.has<Array>(key))
		return table.get<Array>(key);

	Array arr;
	return arr;
}

Object CJsonFile::set_boolean(Object table, LPCSTR key, BOOL value)
{
	return table << key << Boolean(value);
}

Object CJsonFile::set_string(Object table, LPCSTR key, LPCSTR value)
{
	return table << key << String(value);
}

Object CJsonFile::set_number(Object table, LPCSTR key, double value)
{
	return table << key << Number(value);
}

Object CJsonFile::set_object(Object table, LPCSTR key, Object new_table)
{		  
	return table << key << new_table;
}

Object CJsonFile::set_array(Object table, LPCSTR key, Array new_table)
{
	return table << key << new_table;
}

Array CJsonFile::set_object_array(Array table, Object new_table)
{
	return table << new_table;
}

Object CJsonFile::get_object_from_array(Array array, int key, Object new_table)
{
	if (array.has<Object>(key)) 
		return array.get<Object>(key);
	else
	{
		Msg("ћассив содержит неправильную таблицу");
		Object newtab;
		
		return newtab;
	}
}

int CJsonFile::array_size(Array array)
{
	return array.size();
}
 

void CJsonFile::SaveJSON(Object table, LPCSTR file_name, LPCSTR path)
{	
	FS.update_path(directory_path, path, file_name);
	IWriter* file = FS.w_open(directory_path);
	FS.w_close(file);

	std::ofstream ofile(directory_path);

	if (ofile.is_open())
		ofile << table.json().c_str();

	Msg("Save Dir %s", directory_path);

	ofile.close();
}

Object CJsonFile::LoadJSON(Object table, LPCSTR file_name, LPCSTR path)
{
	FS.update_path(directory_path, path, file_name);
	std::ifstream ifile(directory_path);

	if (ifile.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
		table.parse(str);
		Msg("Load SaveName %s", directory_path);
	}

	return table;
}
