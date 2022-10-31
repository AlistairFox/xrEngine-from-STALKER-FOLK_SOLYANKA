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
		Msg("Массив содержит неправильную таблицу");
		Object newtab;
		
		return newtab;
	}
}

int CJsonFile::array_size(Array array)
{
	return array.size();
}
 

void CJsonFile::SaveJSON(LPCSTR file_name, LPCSTR path)
{	
	FS.update_path(directory_path, path, file_name);
	IWriter* file = FS.w_open(directory_path);
	FS.w_close(file);

	std::ofstream ofile(directory_path);

	if (ofile.is_open())
		ofile << json_file.json().c_str();

	//Msg("Save Dir %s", directory_path);

	ofile.close();
}

void CJsonFile::LoadJSON(LPCSTR file_name, LPCSTR path)
{
	FS.update_path(directory_path, path, file_name);
	std::ifstream ifile(directory_path);

	if (ifile.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
		json_file.parse(str);
		//Msg("Load SaveName %s", directory_path);
	}

}

//CLASS CObjectJsonEX

bool CObjectJsonEx::load(LPCSTR file_name, LPCSTR path)
{
	FS.update_path(directory_path, path, file_name);
	std::ifstream ifile(directory_path);
	if (ifile.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
		o::parse(str);
	}
	else
		return false;

	return true;
}

bool CObjectJsonEx::save(LPCSTR file_name, LPCSTR path)
{
	//Сделать Файл Если нету Папок не сделает...
	FS.update_path(directory_path, path, file_name);
	IWriter* file = FS.w_open(directory_path);
	FS.w_close(file);
	//
	std::ofstream ofile(directory_path);
	if (ofile.is_open())
		ofile << o::json().c_str();
	else
	 	return false;

	ofile.close();

	return true;
}

//ARRAY

bool CObjectJsonEx::has_array(LPCSTR key)
{
	return o::has<Array>(key);
}

Array CObjectJsonEx::get_array(LPCSTR key)
{
	return o::get<Array>(key);
}

void CObjectJsonEx::set_array(LPCSTR key, Array value)
{
	return o::import(key, value);
}

//OBJECT

bool CObjectJsonEx::has_object(LPCSTR key)
{
	return o::has<Object>(key);
}

CObjectJsonEx CObjectJsonEx::get_object(LPCSTR key)
{
	Object* obj = &(o::get<Object>(key));
	CObjectJsonEx* ex = reinterpret_cast<CObjectJsonEx*> (obj);
	return *ex;
}

void CObjectJsonEx::set_object(LPCSTR key, CObjectJsonEx table)
{
	Object* obj = reinterpret_cast<Object*> (&table);
 	o::import(key, &obj);
}

//READ OBJECT

bool CObjectJsonEx::has_bool(LPCSTR value)
{
	return o::has<Boolean>(value);
}

bool CObjectJsonEx::has_number(LPCSTR value)
{
	return o::has<Number>(value);
}

bool CObjectJsonEx::has_string(LPCSTR value)
{
	return o::has<String>(value);
}

double CObjectJsonEx::get_number(LPCSTR key)
{
	return o::get<Number>(key);
}

bool CObjectJsonEx::get_bool(LPCSTR key)
{
	return o::get<Boolean>(key);
}

LPCSTR CObjectJsonEx::get_string(LPCSTR key)
{
	return o::get<String>(key).c_str();
}

void CObjectJsonEx::set_string(LPCSTR key, LPCSTR value)
{
	o::import(key, (String)value);
}

void CObjectJsonEx::set_number(LPCSTR key, double value)
{
	o::import(key, (Number)value);
}

void CObjectJsonEx::set_bool(LPCSTR key, bool value)
{
	o::import(key, (Boolean)value);
}