#include "stdafx.h"
#include "xr_ini_ext.h"
#include "..\xrCore\FS_internal.h"
 

BOOL _parse(LPSTR dest, LPCSTR src)
{
	BOOL bInsideSTR = false;
	if (src)
	{
		while (*src)
		{
			if (isspace((u8)*src))
			{
				if (bInsideSTR)
				{
					*dest++ = *src++;
					continue;
				}
				while (*src && isspace(*src))
				{
					++src;
				}
				continue;
			}
			else if (*src == '"')
			{
				bInsideSTR = !bInsideSTR;
			}
			*dest++ = *src++;
		}
	}
	*dest = 0;
	return bInsideSTR;
}

CInifileExt::CInifileExt()
{
}

CInifileExt::CInifileExt(LPCSTR file)
{
}

CInifileExt::~CInifileExt()
{
}

void CInifileExt::ModyfyLine(string_path file_path, LPCSTR file_name, bool save_backup, LPCSTR sec, LPCSTR line, LPCSTR value)
{
	string_path path;
	FS.update_path(path, file_path, file_name);
 
	string512 path_str = { 0 };

	if (save_backup)
	{
		string_path path_new;
		FS.update_path(path_new, "$game_config_backups$", file_name);
		xr_strcpy(path_str, path_new);
	}
	else
	{
		xr_strcpy(path_str, path);
	}
		 
 
	if (!FS.path_exist(file_path))
	{
		Msg("PathNoExist: %s", file_path);
		return;
	}

	IReader* reader = FS.r_open(path);

	xr_vector<Strings> vec_text;
	xr_vector<SectINI> vec_sects;

	string4096 find_sectt;
	bool find_sect = false;

	ItemINI  data;

	while (!reader->eof())
	{
		bool write_str = true;

		string4096 text, orig_text;
		reader->r_string(text, sizeof(text));
		xr_strcpy(orig_text, text);

		_Trim(text);

		if (text[0] && (text[0] == '['))
		{
			*strchr(text, ']') = 0;

			if (xr_strcmp(strlwr(text + 1), sec) == 0)
			{
				find_sect = true;
				xr_strcpy(find_sectt, strlwr(text + 1));
				//Msg("%s == %s", find_sectt, sec);
 			}
		}
		else 
		if (xr_strcmp(sec, find_sectt) == 0)
		{
			string4096			value_raw, str2;
			char* name = text;
			char* t = strchr(name, '=');

			if (t)
			{
				*t = 0;
				_Trim(name);
				++t;
				xr_strcpy(value_raw, sizeof(value_raw), t);
				_parse(str2, value_raw);

				if (xr_strcmp(name, line) == 0)
				{
					//Msg("%s = %s", name, str2);
					data.first = name;
					data.second = value; //str2; 
				
					write_str = false;
				}	
			}
		}

		if (write_str)
		{
			Strings strs;
			xr_strcpy(strs.str, orig_text);
			vec_text.push_back(strs);
		}
		else
		{
			string4096 text_mod;
			xr_strcpy(text_mod, data.first.c_str());
			xr_strcat(text_mod, " = ");
			xr_strcat(text_mod, data.second.c_str());
		
			Strings strs;
			xr_strcpy(strs.str, text_mod);
			vec_text.push_back(strs);
		}
	}

	FS.r_close(reader);

	if (find_sect)
	{
		IWriter* writer = FS.w_open_ex(path_str);

		for (auto strs : vec_text)
			writer->w_string(strs.str);

		FS.w_close(writer);
	}
}

void CInifileExt::LoadIniFile(LPCSTR path, LPCSTR name)
{
	string_path path_to_file;
	string256 file_to = {0};
	xr_strcpy(file_to, path);
	xr_strcat(file_to, name);
	FS.update_path(path_to_file, "$game_config$", file_to);

	IReader* reader = FS.r_open(path_to_file);
	LPCSTR current_section = 0;

	while (!reader->eof())
	{
		string4096 text;
		reader->r_string(text, sizeof(text));
		_Trim(text);
		
		if (text[0] && text[0] == '#' && strstr(text, "#include"))
		{
			vec_includes.push_back(text);
		}
		else if (text[0] && text[0] == '[')
		{
			*strchr(text, ']') = 0;
			string4096 new_text;
			xr_strcpy(new_text, strlwr(text + 1));
			
			SectINI* sec = xr_new<SectINI>();
			sec->name = new_text;
			vec_SECT.push_back(sec);
			current_section = new_text;
		}
		else
		{
			string4096	value_raw, value;
			char* name = text;
			char* t = strchr(name, '=');

			if (t)
			{
				*t = 0;
				_Trim(name);
				++t;
				xr_strcpy(value_raw, sizeof(value_raw), t);
				_parse(value, value_raw);

				ItemINI item;
				item.first = name;
				item.second = value;

				InsertDataToSec(current_section, item);
			}
		}
	}

	FS.r_close(reader);
}

void CInifileExt::InsertDataToSec(LPCSTR sec, ItemINI data)
{
	for (auto section : vec_SECT)
	{
		if (xr_strcmp(section->name, sec) == 0)
		{
			section->item.push_back(data);
		}
	}
}

void CInifileExt::save_as(LPCSTR file)
{
	IWriter* writer = FS.w_open_ex(file);

	for (auto include : vec_includes)
	{
		writer->w_string(include);
	}

	for (auto s : vec_SECT)
	{
		string256 section = {0};
		xr_strcat(section, "[");
		xr_strcat(section, s->name.c_str());
		xr_strcat(section, "]");

		writer->w_string(section);

		for (auto line : s->item)
		{
			string256 line_value = {0};
			xr_strcat(line_value, line.first.c_str());
			xr_strcat(line_value, " = ");
			xr_strcat(line_value, line.second.c_str());

			writer->w_string(line_value);
		}
	}

	FS.w_close(writer);
}

LPCSTR CInifileExt::r_String(LPCSTR section, LPCSTR line)
{
	for (auto sec : vec_SECT)
	{
		if (xr_strcmp(sec->name.c_str(), section) == 0)
		{
			for (auto item : sec->item )
			{
				if (xr_strcmp(item.first, line) == 0)
				{
					return item.second.c_str();
				}
			}
		}
	}

	return 0;
}

int CInifileExt::r_Number(LPCSTR sec, LPCSTR line)
{
	LPCSTR text = r_String(sec, line);
	return atoi(text);
}

Fvector3 CInifileExt::r_Vector3(LPCSTR sec, LPCSTR line)
{
	LPCSTR text = r_String(sec, line);
	Fvector3 vec;
	sscanf(text, "%f,%f,%f", &vec.x, &vec.y, &vec.z);
 	return vec;
}

float CInifileExt::r_Float(LPCSTR sec, LPCSTR line)
{
	LPCSTR text = r_String(sec, line);
	return atof(text);
}

void CInifileExt::w_String(LPCSTR section, LPCSTR line, LPCSTR value)
{
	SectINI* sec = 0;

	for (auto sect : vec_SECT)
	{
		if (xr_strcmp(sect->name, section) == 0)
		{
			sec = sect;
		}
	}

	if (!sec)
	{
		sec = xr_new<SectINI>();
		sec->name = section;
	}
 
	bool has = false;
	for (auto l : sec->item)
	{
		if (xr_strcmp(l.first, line) == 0)
		{
			has = true;
			l.second = value;
		}
	}

	if (!has) 
	{
		ItemINI new_item;
		new_item.first = line;
		new_item.second = value;

		sec->item.push_back(new_item);
	}
}

void CInifileExt::w_Number(LPCSTR sec, LPCSTR line, int value)
{
	string32 tmp;
	w_String(sec, line, itoa(value,tmp,10));
}

void CInifileExt::w_Vector3(LPCSTR sec, LPCSTR line, Fvector3 value)
{
	string256 tmp;
	sprintf(tmp, "%f,%f,%f", &value.x, &value.y, &value.z);
	w_String(sec, line, tmp);
}

void CInifileExt::w_Float(LPCSTR sec, LPCSTR line, float value)
{
	string32 tmp;
	sprintf(tmp, "%f", &value);
	w_String(sec, line, tmp);
}

bool CInifileExt::SectionExist(LPCSTR sect)
{
	for (auto sec : vec_SECT)
	if (xr_strcmp(sec->name, sect) == 0)
		return true;

	return false;
}

bool CInifileExt::LineExist(LPCSTR sec, LPCSTR line)
{
	for (auto sec : vec_SECT)
	for (auto l : sec->item)
	if (xr_strcmp(l.first, line) == 0)
		return true;
 
	return false;
}

CInifileExt::SectINI* CInifileExt::r_section(LPCSTR sec)
{
	for (auto sect : vec_SECT)
	if (xr_strcmp(sect->name, sec) == 0)
		return sect;

	return nullptr;
};
 
void CInifileExt::test_data()
{
	for (auto sec : vec_SECT)
	{
		Msg(";;; START");
		Msg("[%s]", sec->name.c_str());
		for (auto item : sec->item)
		{
			Msg("%s = %s", item.first.c_str(), item.second.c_str());
		}
		Msg(";;; END");
	}
}

void CInifileExt::clear_tables()
{
	for (auto sec : vec_SECT)
		sec->item.clear_and_free();

	vec_SECT.clear_and_free();
}


