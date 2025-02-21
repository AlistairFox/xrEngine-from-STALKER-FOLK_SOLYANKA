#pragma once



class CInifileExt  
{
public: 
	struct ItemINI
	{
		shared_str	first;
		shared_str	second;
		ItemINI() : first(0), second(0) {};
	};

	typedef xr_vector<ItemINI>				ItemsINI;

	struct SectINI
	{
		shared_str name;
		ItemsINI item;
	};

	struct Strings
	{
		string4096 str;
	};

	typedef xr_vector<SectINI*> vec_ini;
	typedef xr_vector<LPCSTR> vec_includ;

protected:
	vec_ini vec_SECT;
	vec_includ vec_includes;

public:

	CInifileExt();
	CInifileExt(LPCSTR file);
	virtual ~CInifileExt();

	void ModyfyLine(string_path file_path, LPCSTR file_name, bool save_backup, LPCSTR sec, LPCSTR line, LPCSTR value);
	void LoadIniFile(LPCSTR file, LPCSTR name);
	void InsertDataToSec(LPCSTR sec, ItemINI data);
	void save_as(LPCSTR file);


	LPCSTR	 r_String(LPCSTR sec, LPCSTR line);
	int		 r_Number(LPCSTR sec, LPCSTR line);
	Fvector3 r_Vector3(LPCSTR sec, LPCSTR line);
	float	 r_Float(LPCSTR sec, LPCSTR line);

	void w_String(LPCSTR sec, LPCSTR line, LPCSTR value);
	void w_Number(LPCSTR sec, LPCSTR line, int value);
	void w_Vector3(LPCSTR sec, LPCSTR line, Fvector3 value);
	void w_Float(LPCSTR sec, LPCSTR line, float value);

	bool SectionExist(LPCSTR sec);
	bool LineExist(LPCSTR sec, LPCSTR line);


 	SectINI* r_section(LPCSTR sec);
	vec_ini get_sections() { return vec_SECT; };


	void test_data();
	void clear_tables();

};

