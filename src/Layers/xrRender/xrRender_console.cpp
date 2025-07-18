#include	"stdafx.h"
#pragma		hdrstop

#include	"xrRender_console.h"
#include	"dxRenderDeviceRender.h"

#define TREE_WIND_EFFECT // configurable tree sway, can be used to have trees sway more during storms or lightly on clear days.
#define DETAIL_RADIUS		

u32 ps_ColorGradingPreset = 0;
xr_token qcolorgrading_preset_token[] = { {"Default", 0}, {"Cold", 1}, {"Filmic 1", 2}, {"Filmic 2", 3}, {"Filmic 3", 4} , {"Hollywood", 5}, {"Vanilla", 6}, {"Vibrant", 7} , {"Warm", 8}, {nullptr, 0} };

// SMAP Control

u32 ps_r2_smapsize = 2048;
xr_token qsmapsize_token[] =
{
	{ "512",						512											},
	{ "1024",						1024										},
	{ "1536",						1536										},
	{ "2048",						2048										},
	{ "2560",						2560										},
	{ "3072",						3072										},
	{ "4096",						4096										},
	{ "8192",						8192										},
	{ 0,							0											}
};

u32			ps_Preset				=	2	;
xr_token							qpreset_token							[ ]={
	{ "Minimum",					0											},
	{ "Low",						1											},
	{ "Default",					2											},
	{ "High",						3											},
	{ "Extreme",					4											},
	{"Ultra_SSR",					5											},
	{ 0,							0											}
};

u32			ps_r_ssao_mode			=	2;
xr_token							qssao_mode_token						[ ]={
	{ "disabled",					0											},
	{ "default",					1											},
	{ "hdao",						2											},
	{ "hbao",						3											},
	{ "hbao_plus",						4											},
	{ 0,							0											}
};

u32			ps_r_sun_shafts				=	2;
xr_token							qsun_shafts_token							[ ]={
	{ "st_opt_off",					0												},
	{ "st_opt_low",					1												},
	{ "st_opt_medium",				2												},
	{ "st_opt_high",				3												},
	{ 0,							0												}
};

//ogse sunshafts
u32 ps_sunshafts_mode = 0;
xr_token sunshafts_mode_token[] = {
	{ "volumetric", 0 },
	{ "screen_space", 1 },
	{ "combine_sunshafts", 2 },
	{ NULL, 0 }
};


u32			ps_r_ssao				=	3;
xr_token							qssao_token									[ ]={
	{ "st_opt_off",					0												},
	{ "st_opt_low",					1												},
	{ "st_opt_medium",				2												},
	{ "st_opt_high",				3												},
	{ "st_opt_ultra",				4												},
	{ 0,							0												}
};

u32			ps_r_sun_quality		=	1;			//	=	0;
xr_token							qsun_quality_token							[ ]={
	{ "st_opt_low",					0												},
	{ "st_opt_medium",				1												},
	{ "st_opt_high",				2												},
	{ "st_opt_ultra",				3												},
	{ "st_opt_extreme",				4												},
	{ 0,							0												}
};

u32			ps_r3_msaa				=	0;			//	=	0;
xr_token							qmsaa_token							[ ]={
	{ "st_opt_off",					0												},
	{ "2x",							1												},
	{ "4x",							2												},
//	{ "8x",							3												},
	{ 0,							0												}
};

u32			ps_r3_msaa_atest		=	0;			//	=	0;
xr_token							qmsaa__atest_token					[ ]={
	{ "st_opt_off",					0												},
	{ "st_opt_atest_msaa_dx10_0",	1												},
	{ "st_opt_atest_msaa_dx10_1",	2												},
	{ 0,							0												}
};

u32			ps_r3_minmax_sm			=	3;			//	=	0;
xr_token							qminmax_sm_token					[ ]={
	{ "off",						0												},
	{ "on",							1												},
	{ "auto",						2												},
	{ "autodetect",					3												},
	{ 0,							0												}
};

//	�Off�
//	�DX10.0 style [Standard]�
//	�DX10.1 style [Higher quality]�

// Common
extern int			psSkeletonUpdate;
extern float		r__dtex_range;

Flags32 ps_r__common_flags = {/*RFLAG_NO_RAM_TEXTURES*/ }; // All renders

int			ps_r__LightSleepFrames		= 10	;

float		ps_r__Detail_l_ambient		= 0.9f	;
float		ps_r__Detail_l_aniso		= 0.25f	;
float		ps_r__Detail_density		= 0.3f	;
float		ps_r__Detail_height = 1.0f;
float		ps_r__Detail_rainbow_hemi	= 0.75f	;

float		ps_r__Tree_w_rot			= 10.0f	;
float		ps_r__Tree_w_speed			= 1.00f	;
float		ps_r__Tree_w_amp			= 0.005f;
Fvector		ps_r__Tree_Wave				= {.1f, .01f, .11f};
float		ps_r__Tree_SBC				= 1.5f	;	// scale bias correct

float		ps_r__WallmarkTTL			= 50.f	;
float		ps_r__WallmarkSHIFT			= 0.0001f;
float		ps_r__WallmarkSHIFT_V		= 0.0001f;

float		ps_r__GLOD_ssa_start		= 256.f	;
float		ps_r__GLOD_ssa_end			=  64.f	;
float		ps_r__LOD					=  0.75f	;
//. float		ps_r__LOD_Power				=  1.5f	;
float		ps_r__ssaDISCARD			=  3.5f	;					//RO
float		ps_r__ssaDONTSORT			=  32.f	;					//RO
float		ps_r__ssaHZBvsTEX			=  96.f	;					//RO

int			ps_r__tf_Anisotropic		= 8		;
float ps_r__tf_Mipbias = 0.0f;

// R1
float		ps_r1_ssaLOD_A				= 64.f	;
float		ps_r1_ssaLOD_B				= 48.f	;
float		ps_r1_tf_Mipbias			= 0.0f	;
Flags32		ps_r1_flags					= { R1FLAG_DLIGHTS };		// r1-only
float		ps_r1_lmodel_lerp			= 0.1f	;
float		ps_r1_dlights_clip			= 40.f	;
float		ps_r1_pps_u					= 0.f	;
float		ps_r1_pps_v					= 0.f	;
extern float			hbao_plus_radius = 1.5;
extern float			hbao_plus_bias = 0.15;
extern float hbao_plus_power_exponent = 1.5;
extern float hbao_plus_blur_sharp = 32;
extern Fvector3 sight_color = { 0,0,0};

// R1-specific
int			ps_r1_GlowsPerFrame			= 16	;					// r1-only
float		ps_r1_fog_luminance			= 1.1f	;					// r1-only
int			ps_r1_SoftwareSkinning		= 0		;					// r1-only

// R2
float		ps_r2_ssaLOD_A				= 64.f	;
float		ps_r2_ssaLOD_B				= 48.f	;
float		ps_r2_tf_Mipbias			= 0.0f	;

//r4 only
Flags32 ps_r4_ssr_flags = {
 R4_FLAG_SSR_USE
|R4_FLAG_MAX_QUALITY_SHADERS
|R4_FLAG_USE_ADVANCED_SHADERS
};

// R2-specific
Flags32		ps_r2_ls_flags				= { R2FLAG_SUN 
	//| R2FLAG_SUN_IGNORE_PORTALS
	| R2FLAG_EXP_DONT_TEST_UNSHADOWED 
	| R2FLAG_USE_NVSTENCIL | R2FLAG_EXP_SPLIT_SCENE 
	| R3FLAG_DYN_WET_SURF
	| R3FLAG_VOLUMETRIC_SMOKE
	//| R3FLAG_MSAA 
	//| R3FLAG_MSAA_OPT
	|R2FLAG_DETAIL_BUMP
	|R2FLAG_DOF
	|R2FLAG_SOFT_PARTICLES
	|R2FLAG_SOFT_WATER
	|R2FLAG_STEEP_PARALLAX
	|R2FLAG_SUN_FOCUS
	|R2FLAG_SUN_TSM
	|R2FLAG_TONEMAP
	//|R2FLAG_VOLUMETRIC_LIGHTS
	};	// r2-only

Flags32		ps_r2_ls_flags_ext			= {
		/*R2FLAGEXT_SSAO_OPT_DATA |*/ R2FLAGEXT_SSAO_HALF_DATA
		|R2FLAGEXT_ENABLE_TESSELLATION
	};

int			ps_no_scale_on_fade = 0;
float		ps_r2_df_parallax_h			= 0.02f;
float		ps_r2_df_parallax_range		= 75.f;
float		ps_r2_tonemap_middlegray = 1.3f;
float		ps_r2_tonemap_adaptation = 10.f;
float		ps_r2_tonemap_low_lum = 0.5f;
float		ps_r2_tonemap_amount = 1.0f;
float		ps_r2_ls_bloom_kernel_g		= 3.f;				// r2-only
float		ps_r2_ls_bloom_kernel_b = 1.0f;
float		ps_r2_ls_bloom_speed = 50.f;
float		ps_r2_ls_bloom_kernel_scale = .9f; // gauss
float		ps_r2_ls_dsm_kernel			= .7f;				// r2-only
float		ps_r2_ls_psm_kernel			= .7f;				// r2-only
float		ps_r2_ls_ssm_kernel			= .7f;				// r2-only
float		ps_r2_ls_bloom_threshold = .03f;
Fvector		ps_r2_aa_barier				= { .8f, .1f, 0};	// r2-only
Fvector		ps_r2_aa_weight				= { .25f,.25f,0};	// r2-only
float		ps_r2_aa_kernel				= .5f;				// r2-only
float		ps_r2_mblur					= .0f;				// .5f
int			ps_r2_GI_depth				= 1;				// 1..5
int			ps_r2_GI_photons			= 16;				// 8..64
float		ps_r2_GI_clip				= EPS_L;			// EPS
float		ps_r2_GI_refl				= .9f;				// .9f
float		ps_r2_ls_depth_scale		= 1.00001f;			// 1.00001f
float		ps_r2_ls_depth_bias			= -0.001f;			// -0.0001f
float		ps_r2_ls_squality			= 1.0f;				// 1.00f
float		ps_r2_sun_tsm_projection	= 0.3f;			// 0.18f
float		ps_r2_sun_tsm_bias			= -0.01f;			// 
float		ps_r2_sun_near				= 20.f;				// 12.0f

extern float OLES_SUN_LIMIT_27_01_07;	//	actually sun_far

float		ps_r2_sun_near_border		= 0.75f;			// 1.0f
float		ps_r2_sun_depth_far_scale	= 1.00000f;			// 1.00001f
float		ps_r2_sun_depth_far_bias	= -0.00002f;			// -0.0000f
float		ps_r2_sun_depth_near_scale	= 1.0000f;			// 1.00001f
float		ps_r2_sun_depth_near_bias	= 0.00001f;		// -0.00005f
float		ps_r2_sun_lumscale			= 1.0f;				// 1.0f
float		ps_r2_sun_lumscale_hemi		= 1.0f;				// 1.0f
float		ps_r2_sun_lumscale_amb		= 1.0f;
float		ps_r2_gmaterial				= 2.2f;				// 
float		ps_r2_zfill					= 0.25f;				// .1f

float		ps_r2_dhemi_sky_scale		= 0.08f;				// 1.5f
float		ps_r2_dhemi_light_scale     = 0.2f	;
float		ps_r2_dhemi_light_flow      = 0.1f	;
int			ps_r2_dhemi_count			= 5;				// 5
int			ps_r2_wait_sleep			= 0;

float		ps_r2_lt_smooth				= 1.f;				// 1.f
float		ps_r2_slight_fade			= 0.5f;				// 1.f

// Screen Space Shaders Stuff
// Anomaly
// Anomaly
float ps_r2_img_exposure = 1.0f; // r2-only
float ps_r2_img_gamma = 1.0f; // r2-only
float ps_r2_img_saturation = 1.0f; // r2-only
Fvector ps_r2_img_cg = { .0f, .0f, .0f }; // r2-only


// Ascii1457's Screen Space Shaders
Fvector3 ps_ssfx_shadow_cascades = { 40.f, 60.f, 160.f };
Fvector4 ps_ssfx_grass_shadows = { 2.f, 1.f, 30.0f, .0f };
Fvector4 ps_ssfx_grass_interactive = { 1.0f, 8.f, 200.0f, 1.0f };
Fvector4 ps_ssfx_int_grass_params_1 = { 0.5f, 1.0f, 1.0f, 50.0f };
Fvector4 ps_ssfx_int_grass_params_2 = { 1.0f, 5.0f, 1.0f, 5.0f };
Fvector4 ps_ssfx_hud_drops_1 = { 1.0f, 1.0f, 30.f, .05f }; // Anim Speed, Int, Reflection, Refraction
Fvector4 ps_ssfx_hud_drops_2 = { .1f, 1.2f, 0.0f, 2.0f }; // Density, Size, Extra Gloss, Gloss
Fvector4 ps_ssfx_blood_decals = { 1.f, 0.7f, 0.f, 0.f };
Fvector4 ps_ssfx_rain_1 = { 2.0f, 0.1f, 0.5f, 2.f }; // Len, Width, Speed, Quality
Fvector4 ps_ssfx_rain_2 = { 0.3f, 2.0f, 1.0f, 0.5f }; // Alpha, Brigthness, Refraction, Reflection
Fvector4 ps_ssfx_rain_3 = { 0.01f, 1.0f, 0.0f, 0.0f }; // Alpha, Refraction ( Splashes ) - Yohji: Alpha was edited (0.5->0.01f) due to a bug with transparency and other particles.
Fvector4 ps_ssfx_wind_grass = { 9.5f, 1.4f, 1.5f, 0.4f };
Fvector4 ps_ssfx_wind_trees = { 11.0f, 0.15f, 0.5f, 0.25f };

int ps_ssfx_ssr_quality = 2; // Quality
Fvector4 ps_ssfx_ssr = { 1.0f, 0.4f, 0.6f, 0.0f }; // Res, Blur, Temp, Noise
Fvector4 ps_ssfx_ssr_2 = { 1.0f, 1.3f, 2.0f, 0.1f }; // Quality, Fade, Int, Wpn Int
Fvector4 ps_ssfx_volumetric = { 0, 1.0f, 3.0f, 8.0f }; // x - ? y - ? z - ? w - Resolution
Fvector3 ps_ssfx_shadow_bias = { 0.4f, 0.03f, 0.0f };

Fvector4 ps_ssfx_terrain_quality = { 6, 0, 0, 0 };
Fvector4 ps_ssfx_terrain_offset = { 0, 0, 0, 0 };

Fvector3	ps_r2_dof					= Fvector3().set(-1.25f, 1.4f, 10000.f);
float		ps_r2_dof_sky				= 30;				//	distance to sky
float		ps_r2_dof_kernel_size		= 5.0f;						//	7.0f

float		ps_r3_dyn_wet_surf_near		= 10.f;				// 10.0f
float		ps_r3_dyn_wet_surf_far		= 30.f;				// 30.0f
int			ps_r3_dyn_wet_surf_sm_res	= 256;				// 256

int			ps_r__detail_radius = 100;
#ifdef DETAIL_RADIUS // ���������� �������� ��������� �����
u32 dm_size = 24;
u32 dm_cache1_line = 12; //dm_size*2/dm_cache1_count
u32 dm_cache_line = 49; //dm_size+1+dm_size
u32 dm_cache_size = 2401; //dm_cache_line*dm_cache_line
float dm_fade = 47.5; //float(2*dm_size)-.5f;
u32 dm_current_size = 24;
u32 dm_current_cache1_line = 12; //dm_current_size*2/dm_cache1_count
u32 dm_current_cache_line = 49; //dm_current_size+1+dm_current_size
u32 dm_current_cache_size = 2401; //dm_current_cache_line*dm_current_cache_line
float dm_current_fade = 47.5; //float(2*dm_current_size)-.5f;
#endif
float ps_current_detail_density = 1.f;
float ps_current_detail_height = 1.2f;


//ogse sunshafts
float		ps_r2_ss_sunshafts_length = 1.f;
float		ps_r2_ss_sunshafts_radius = 1.f;

//Pseudopbr
float ps_r3_pbr_intensity = 25.0f;
float ps_r3_pbr_roughness = 0.5f;
Flags32	ps_r3_pbr_flags = { R_FLAG_PSEUDOPBR };
//Geometry optimization from Anomaly
int opt_static = 0;
int opt_dynamic = 0;

//SFZ Lens Flares
int ps_r2_lfx = 0;



//Static on dx11
Flags32	ps_r2_static_flags = {
	R2FLAG_USE_BUMP
};

Flags32 psDeviceFlags2 = { 0 };


Flags32		ps_actor_shadow_flags = { 1 };



float ps_r2_gloss_factor = 10.0f;
float ps_r2_gloss_min = 0.0f;
#ifndef _EDITOR
#include	"../../xrEngine/xr_ioconsole.h"
#include	"../../xrEngine/xr_ioc_cmd.h"

#include "../xrRenderDX10/StateManager/dx10SamplerStateCache.h"

class CCC_ssfx_cascades : public CCC_Vector3
	 {
public:
	void apply()
		 {
			RImplementation.init_cascades();
			 }
	
		CCC_ssfx_cascades(LPCSTR N, Fvector3* V, const Fvector3 _min, const Fvector3 _max) : CCC_Vector3(N, V, _min, _max)
		 {
		};
	
		virtual void Execute(LPCSTR args)
		 {
		CCC_Vector3::Execute(args);
		apply();
		}
	
		virtual void GetStatus(TStatus& S)
		 {
		CCC_Vector3::Status(S);
		apply();
		}
	 };


//-----------------------------------------------------------------------

class CCC_detail_radius : public CCC_Integer
{
public:
	void	apply()
	{
		dm_current_size = iFloor((float)ps_r__detail_radius / 4) * 2;
		dm_current_cache1_line = dm_current_size * 2 / 4;		// assuming cache1_count = 4
		dm_current_cache_line = dm_current_size + 1 + dm_current_size;
		dm_current_cache_size = dm_current_cache_line * dm_current_cache_line;
		dm_current_fade = float(2 * dm_current_size) - .5f;
	}
	CCC_detail_radius(LPCSTR N, int* V, int _min = 0, int _max = 999) : CCC_Integer(N, V, _min, _max)
	{};
	virtual void Execute(LPCSTR args)
	{
		CCC_Integer::Execute(args);
		apply();
	}
	virtual void	Status(TStatus& S)
	{
		CCC_Integer::Status(S);
	}
};

class CCC_tf_Aniso		: public CCC_Integer
{
public:
	void	apply	()	{
		if (0==HW.pDevice)	return	;
		int	val = *value;	clamp(val,1,16);
		SSManager.SetMaxAnisotropy(val);

	}
	CCC_tf_Aniso(LPCSTR N, int*	v) : CCC_Integer(N, v, 1, 16)		{ };
	virtual void Execute	(LPCSTR args)
	{
		CCC_Integer::Execute	(args);
		apply					();
	}
	virtual void	Status	(TStatus& S)
	{	
		CCC_Integer::Status		(S);
		apply					();
	}
};
class CCC_tf_MipBias: public CCC_Float
{
public:
	void	apply	()	{
		if (0==HW.pDevice)	return	;

		SSManager.SetMipLODBias(*value);
		//	TODO: DX10: Implement mip bias control
		//VERIFY(!"apply not implmemented.");

	}


	CCC_tf_MipBias(LPCSTR N, float*	v) : CCC_Float(N, v, -3.f, +3.f)	{ };
	virtual void Execute(LPCSTR args)
	{
		CCC_Float::Execute	(args);
		apply				();
	}
	virtual void	Status	(TStatus& S)
	{	
		CCC_Float::Status	(S);
		apply				();
	}
};

class CCC_R2GM		: public CCC_Float
{
public:
	CCC_R2GM(LPCSTR N, float*	v) : CCC_Float(N, v, 0.f, 4.f) { *v = 0; };
	virtual void	Execute	(LPCSTR args)
	{
		if (0==xr_strcmp(args,"on"))	{
			ps_r2_ls_flags.set	(R2FLAG_GLOBALMATERIAL,TRUE);
		} else if (0==xr_strcmp(args,"off"))	{
			ps_r2_ls_flags.set	(R2FLAG_GLOBALMATERIAL,FALSE);
		} else {
			CCC_Float::Execute	(args);
			if (ps_r2_ls_flags.test(R2FLAG_GLOBALMATERIAL))	{
				static LPCSTR	name[4]	=	{ "oren", "blin", "phong", "metal" };
				float	mid		= *value	;
				int		m0		= iFloor(mid)	% 4;
				int		m1		= (m0+1)		% 4;
				float	frc		= mid - float(iFloor(mid));
				Msg		("* material set to [%s]-[%s], with lerp of [%f]",name[m0],name[m1],frc);
			}
		}
	}
};
class CCC_Screenshot : public IConsole_Command
{
public:
	CCC_Screenshot(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		if (g_dedicated_server)
			return;

		string_path	name;	name[0]=0;
		sscanf		(args,"%s",	name);
		LPCSTR		image	= xr_strlen(name)?name:0;
		::Render->Screenshot(IRender_interface::SM_NORMAL,image);
	}
};

class CCC_RestoreQuadIBData : public IConsole_Command
{
public:
	CCC_RestoreQuadIBData(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		RCache.RestoreQuadIBData();
	}
};

class CCC_ModelPoolStat : public IConsole_Command
{
public:
	CCC_ModelPoolStat(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) {
		RImplementation.Models->dump();
	}
};

class	CCC_SSAO_Mode		: public CCC_Token
{
public:
	CCC_SSAO_Mode(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N,V,T)	{}	;

	virtual void	Execute	(LPCSTR args)	{
		CCC_Token::Execute	(args);
				
		switch	(*value)
		{
			case 0:
			{
				ps_r_ssao = 0;
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_HBAO_PLUS, 0);
				break;
			}
			case 1:
			{
				if (ps_r_ssao==0)
				{
					ps_r_ssao = 1;
				}
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HALF_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_HBAO_PLUS, 0);
				break;
			}
			case 2:
			{
				if (ps_r_ssao==0)
				{
					ps_r_ssao = 1;
				}
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 1);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_OPT_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HALF_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_HBAO_PLUS, 0);
				break;
			}
			case 3:
			{
				if (ps_r_ssao==0)
				{
					ps_r_ssao = 1;
				}
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 1);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_OPT_DATA, 1);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_HBAO_PLUS, 0);
				break;
			}
			case 4:
			{
				ps_r_ssao == 0;

				ps_r2_ls_flags_ext.set(R2FLAGEXT_HBAO_PLUS, 1);

				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_OPT_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HALF_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);

			}break;
		}
	}
};

//-----------------------------------------------------------------------
class	CCC_Preset		: public CCC_Token
{
public:
	CCC_Preset(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N,V,T)	{}	;

	virtual void	Execute	(LPCSTR args)	{
		CCC_Token::Execute	(args);
		string_path		_cfg;
		string_path		cmd;
		
		switch	(*value)	{
			case 0:		xr_strcpy(_cfg, "rspec_minimum.ltx");	break;
			case 1:		xr_strcpy(_cfg, "rspec_low.ltx");		break;
			case 2:		xr_strcpy(_cfg, "rspec_default.ltx");	break;
			case 3:		xr_strcpy(_cfg, "rspec_high.ltx");		break;
			case 4:		xr_strcpy(_cfg, "rspec_extreme.ltx");	break;
			case 5:		xr_strcpy(_cfg, "rspec_ultra_ssr.ltx"); break;
		}
		FS.update_path			(_cfg,"$game_config$",_cfg);
		strconcat				(sizeof(cmd),cmd,"cfg_load", " ", _cfg);
		Console->Execute		(cmd);
	}
};


class CCC_ColorGrading_Preset : public CCC_Token
{
public:
	CCC_ColorGrading_Preset(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N, V, T) {};

	virtual void Execute(LPCSTR args)
	{
		CCC_Token::Execute(args);
		string_path _cfg;
		string_path cmd;

		switch (*value)
		{
		case 0: xr_strcpy(_cfg, "grading_default.ltx"); break;
		case 1: xr_strcpy(_cfg, "grading_cold.ltx"); break;
		case 2: xr_strcpy(_cfg, "grading_filmic01.ltx"); break;
		case 3: xr_strcpy(_cfg, "grading_filmic02.ltx"); break;
		case 4: xr_strcpy(_cfg, "grading_filmic03.ltx"); break;
		case 5: xr_strcpy(_cfg, "grading_hollywood.ltx"); break;
		case 6: xr_strcpy(_cfg, "grading_vanilla.ltx"); break;
		case 7: xr_strcpy(_cfg, "grading_vibrant.ltx"); break;
		case 8: xr_strcpy(_cfg, "grading_warm.ltx"); break;
		}
		FS.update_path(_cfg, "$game_config$", _cfg);
		strconcat(sizeof(cmd), cmd, "cfg_load", " ", _cfg);
		Console->Execute(cmd);
	}
};

#include "r__pixel_calculator.h"
class CCC_BuildSSA : public IConsole_Command
{
public:
	CCC_BuildSSA(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) 
	{

	}
};


class CCC_DofFar : public CCC_Float
{
public:
	CCC_DofFar(LPCSTR N, float* V, float _min=0.0f, float _max=10000.0f) 
		: CCC_Float( N, V, _min, _max){}

	virtual void Execute(LPCSTR args) 
	{
		float v = float(atof(args));

		if (v<ps_r2_dof.y+0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value greater or equal to r2_dof_focus+0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_focus");
		}
		else
		{
			CCC_Float::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
		}
	}

	//	CCC_Dof should save all data as well as load from config
	virtual void	Save	(IWriter *F)	{;}
};

class CCC_DofNear : public CCC_Float
{
public:
	CCC_DofNear(LPCSTR N, float* V, float _min=0.0f, float _max=10000.0f) 
		: CCC_Float( N, V, _min, _max){}

	virtual void Execute(LPCSTR args) 
	{
		float v = float(atof(args));

		if (v>ps_r2_dof.y-0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value less or equal to r2_dof_focus-0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_focus");
		}
		else
		{
			CCC_Float::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
		}
	}

	//	CCC_Dof should save all data as well as load from config
	virtual void	Save	(IWriter *F)	{;}
};

class CCC_DofFocus : public CCC_Float
{
public:
	CCC_DofFocus(LPCSTR N, float* V, float _min=0.0f, float _max=10000.0f) 
		: CCC_Float( N, V, _min, _max){}

	virtual void Execute(LPCSTR args) 
	{
		float v = float(atof(args));

		if (v>ps_r2_dof.z-0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value less or equal to r2_dof_far-0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_far");
		}
		else if (v<ps_r2_dof.x+0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value greater or equal to r2_dof_far-0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_near");
		}
		else{
			CCC_Float::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
			}
	}

	//	CCC_Dof should save all data as well as load from config
	virtual void	Save	(IWriter *F)	{;}
};

class CCC_Dof : public CCC_Vector3
{
public:
	CCC_Dof(LPCSTR N, Fvector* V, const Fvector _min, const Fvector _max) : 
	  CCC_Vector3(N, V, _min, _max) {;}

	virtual void	Execute	(LPCSTR args)
	{
		Fvector v;
		if (3!=sscanf(args,"%f,%f,%f",&v.x,&v.y,&v.z))	
			InvalidSyntax(); 
		else if ( (v.x > v.y-0.1f) || (v.z < v.y+0.1f))
		{
			InvalidSyntax();
			Msg("x <= y - 0.1");
			Msg("y <= z - 0.1");
		}
		else
		{
			CCC_Vector3::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
		}
	}
	virtual void	Status	(TStatus& S)
	{	
		xr_sprintf	(S,"%f,%f,%f",value->x,value->y,value->z);
	}
	virtual void	Info	(TInfo& I)
	{	
		xr_sprintf(I,"vector3 in range [%f,%f,%f]-[%f,%f,%f]",min.x,min.y,min.z,max.x,max.y,max.z);
	}

};

class CCC_DumpResources : public IConsole_Command
{
public:
	CCC_DumpResources(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) {
		RImplementation.Models->dump();
		dxRenderDeviceRender::Instance().Resources->Dump(false);
	}
};

//	Allow real-time fog config reload
#if	(RENDER == R_R4)
#ifdef	DEBUG

#include "../xrRenderDX10/3DFluid/dx103DFluidManager.h"

class CCC_Fog_Reload : public IConsole_Command
{
public:
	CCC_Fog_Reload(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) 
	{
		FluidManager.UpdateProfiles();
	}
};
#endif	//	DEBUG
#endif	//	(RENDER == R_R3) || (RENDER == R_R4)

//-----------------------------------------------------------------------

extern int DetailScale = true;


void		xrRender_initconsole	()
{


	CMD3(CCC_Preset,	"_preset",				&ps_Preset,	qpreset_token	);

	CMD4(CCC_Integer, "r_detail_scale", &DetailScale, 0, 1);

	CMD4(CCC_Integer,	"rs_skeleton_update",	&psSkeletonUpdate,	2,		128	);
#ifdef	DEBUG
	CMD1(CCC_DumpResources,		"dump_resources");
#endif	//	 DEBUG

	CMD4(CCC_Float,		"r__dtex_range",		&r__dtex_range,		5,		175	);

// Common
	CMD1(CCC_Screenshot,"screenshot"			);

	//	Igor: just to test bug with rain/particles corruption
	CMD1(CCC_RestoreQuadIBData,	"r_restore_quad_ib_data");
#ifdef DEBUG
	CMD1(CCC_BuildSSA,	"build_ssa"				);
	CMD4(CCC_Integer,	"r__lsleep_frames",		&ps_r__LightSleepFrames,	4,		30		);
	CMD4(CCC_Float,		"r__ssa_glod_start",	&ps_r__GLOD_ssa_start,		128,	512		);
	CMD4(CCC_Float,		"r__ssa_glod_end",		&ps_r__GLOD_ssa_end,		16,		96		);
	CMD4(CCC_Float,		"r__wallmark_shift_pp",	&ps_r__WallmarkSHIFT,		0.0f,	1.f		);
	CMD4(CCC_Float,		"r__wallmark_shift_v",	&ps_r__WallmarkSHIFT_V,		0.0f,	1.f		);
	CMD1(CCC_ModelPoolStat,"stat_models"		);
#endif // DEBUG
	CMD4(CCC_Float,		"r__wallmark_ttl",		&ps_r__WallmarkTTL,			1.0f,	5.f*60.f);

	Fvector	tw_min,tw_max;
	
	CMD4(CCC_Float, "r__geometry_lod", &ps_r__LOD, 0.1f, 1.5f); //AVO: extended from 1.2f to 3.f
//.	CMD4(CCC_Float,		"r__geometry_lod_pow",	&ps_r__LOD_Power,			0,		2		);

//.	CMD4(CCC_Float,		"r__detail_density",	&ps_r__Detail_density,		.05f,	0.99f	);
	CMD4(CCC_Float, "r__detail_density", &ps_current_detail_density, 1.f, 1.f);
	CMD4(CCC_Float, "r__detail_scale", &ps_current_detail_height, 1.2f, 1.2f);


	CMD4(CCC_Float,		"r__detail_l_ambient",	&ps_r__Detail_l_ambient,	.5f,	.95f	);
	CMD4(CCC_Float,		"r__detail_l_aniso",	&ps_r__Detail_l_aniso,		.1f,	.5f		);
#ifdef DEBUG
	CMD4(CCC_Float,		"r__d_tree_w_amp",		&ps_r__Tree_w_amp,			.001f,	1.f		);
	CMD4(CCC_Float,		"r__d_tree_w_rot",		&ps_r__Tree_w_rot,			.01f,	100.f	);
	CMD4(CCC_Float,		"r__d_tree_w_speed",	&ps_r__Tree_w_speed,		1.0f,	10.f	);

	tw_min.set			(EPS,EPS,EPS);
	tw_max.set			(2,2,2);
	CMD4(CCC_Vector3,	"r__d_tree_wave",		&ps_r__Tree_Wave,			tw_min, tw_max	);
#endif // DEBUG

	CMD2(CCC_tf_Aniso,	"r__tf_aniso",			&ps_r__tf_Anisotropic		); //	{1..16}

	// R1
	CMD4(CCC_Float,		"r1_ssa_lod_a",			&ps_r1_ssaLOD_A,			16,		96		);
	CMD4(CCC_Float,		"r1_ssa_lod_b",			&ps_r1_ssaLOD_B,			16,		64		);
	CMD4(CCC_Float,		"r1_lmodel_lerp",		&ps_r1_lmodel_lerp,			0,		0.333f	);
	CMD2(CCC_tf_MipBias, "r__tf_mipbias", &ps_r__tf_Mipbias); // {-3 +3}
	CMD3(CCC_Mask,		"r1_dlights",			&ps_r1_flags,				R1FLAG_DLIGHTS	);
	CMD4(CCC_Float,		"r1_dlights_clip",		&ps_r1_dlights_clip,		10.f,	150.f	);
	CMD4(CCC_Float,		"r1_pps_u",				&ps_r1_pps_u,				-1.f,	+1.f	);
	CMD4(CCC_Float,		"r1_pps_v",				&ps_r1_pps_v,				-1.f,	+1.f	);

	// R1-specific
	CMD4(CCC_Integer,	"r1_glows_per_frame",	&ps_r1_GlowsPerFrame,		2,		32		);

	CMD4(CCC_Float,		"r1_fog_luminance",		&ps_r1_fog_luminance,		0.2f,	5.f	);

	// Software Skinning
	// 0 - disabled (renderer can override)
	// 1 - enabled
	// 2 - forced hardware skinning (renderer can not override)
	CMD4(CCC_Integer,	"r1_software_skinning",	&ps_r1_SoftwareSkinning,	0,		2	);

	// R2
	CMD4(CCC_Float,		"r2_ssa_lod_a",			&ps_r2_ssaLOD_A,			16,		96		);
	CMD4(CCC_Float,		"r2_ssa_lod_b",			&ps_r2_ssaLOD_B,			32,		40		);
	CMD3(CCC_ColorGrading_Preset, "_colorgrading_preset", &ps_ColorGradingPreset, qcolorgrading_preset_token);

	// R2-specific
	CMD2(CCC_R2GM,		"r2em",					&ps_r2_gmaterial							);
	CMD3(CCC_Mask,		"r2_tonemap",			&ps_r2_ls_flags,			R2FLAG_TONEMAP	);
	CMD4(CCC_Float,		"r2_tonemap_middlegray",&ps_r2_tonemap_middlegray,	1.0f,	1.0f	);
	CMD4(CCC_Float,		"r2_tonemap_adaptation",&ps_r2_tonemap_adaptation,	1.f,	1.f	);
	CMD4(CCC_Float,		"r2_tonemap_lowlum",	&ps_r2_tonemap_low_lum,		0.0001f, 0.0002f	);
	CMD4(CCC_Float,		"r2_tonemap_amount",	&ps_r2_tonemap_amount,		0.6f,0.7f	);
	CMD4(CCC_Float,		"r2_ls_bloom_kernel_scale",&ps_r2_ls_bloom_kernel_scale,	0.5f,	2.f);
	CMD4(CCC_Float,		"r2_ls_bloom_kernel_g",	&ps_r2_ls_bloom_kernel_g,	1.f,	7.f		);
	CMD4(CCC_Float,		"r2_ls_bloom_kernel_b",	&ps_r2_ls_bloom_kernel_b,	0.01f,	1.f		);
	CMD4(CCC_Float,		"r2_ls_bloom_threshold",&ps_r2_ls_bloom_threshold,	0.f,	1.f		);
	CMD4(CCC_Float,		"r2_ls_bloom_speed",	&ps_r2_ls_bloom_speed,		0.f,	100.f	);
	CMD3(CCC_Mask,		"r2_ls_bloom_fast",		&ps_r2_ls_flags,			R2FLAG_FASTBLOOM);
	CMD4(CCC_Float,		"r2_ls_dsm_kernel",		&ps_r2_ls_dsm_kernel,		.1f,	3.f		);
	CMD4(CCC_Float,		"r2_ls_psm_kernel",		&ps_r2_ls_psm_kernel,		.1f,	3.f		);
	CMD4(CCC_Float,		"r2_ls_ssm_kernel",		&ps_r2_ls_ssm_kernel,		.1f,	3.f		);
	CMD4(CCC_Float,		"r2_ls_squality",		&ps_r2_ls_squality,			.5f,	1.f		);

	CMD3(CCC_Mask,		"r2_zfill",				&ps_r2_ls_flags,			R2FLAG_ZFILL	);
	CMD4(CCC_Float,		"r2_zfill_depth",		&ps_r2_zfill,				.001f,	.5f		);
	CMD3(CCC_Mask,		"r2_allow_r1_lights",	&ps_r2_ls_flags,			R2FLAG_R1LIGHTS	);

	CMD3(CCC_Mask, "r__actor_shadow", &ps_actor_shadow_flags, RFLAG_ACTOR_SHADOW);  //Swartz

	tw_min.set(0, 0, 0);
	tw_max.set(1, 1, 1);
	Fvector4 tw2_min = { -100.f, -100.f, -100.f, -100.f };
	Fvector4 tw2_max = { 100.f, 100.f, 100.f, 100.f };

	CMD4(CCC_Vector3, "r__color_grading", &ps_r2_img_cg, tw_min, tw_max);
	// Anomaly
	CMD4(CCC_Float, "r__exposure", &ps_r2_img_exposure, 0.5f, 4.0f);
	CMD4(CCC_Float, "r__gamma", &ps_r2_img_gamma, 0.5f, 2.2f);
	CMD4(CCC_Float, "r__saturation", &ps_r2_img_saturation, 0.0f, 2.0f);

	CMD3(CCC_Token, "r2_smap_size", &ps_r2_smapsize, qsmapsize_token);


	//no ram textures should be enabled by default on r3/r4
	if (RENDER == R_R4) ps_r__common_flags.set(RFLAG_NO_RAM_TEXTURES, TRUE);

	CMD3(CCC_Mask, "r__no_ram_textures", &ps_r__common_flags, RFLAG_NO_RAM_TEXTURES);

	CMD4(CCC_Float,		"r2_gloss_factor",		&ps_r2_gloss_factor,		.0f,	10.f	);
	CMD4(CCC_Float, "r2_gloss_min", &ps_r2_gloss_min, .001f, 1.0f);

#ifdef DEBUG
	CMD3(CCC_Mask,		"r2_use_nvdbt",			&ps_r2_ls_flags,			R2FLAG_USE_NVDBT);
#endif // DEBUG

	CMD3(CCC_Mask, "screen_space_reflections", &ps_r4_ssr_flags, R4_FLAG_SSR_USE);
	CMD3(CCC_Mask, "r__use_max_quality_shaders", &ps_r4_ssr_flags, R4_FLAG_MAX_QUALITY_SHADERS);
	CMD3(CCC_Mask, "r__use_advanced_shaders", &ps_r4_ssr_flags, R4_FLAG_USE_ADVANCED_SHADERS);

	CMD3(CCC_Mask,		"r2_sun",				&ps_r2_ls_flags,			R2FLAG_SUN		);
	CMD3(CCC_Mask,		"r2_sun_details",		&ps_r2_ls_flags,			R2FLAG_SUN_DETAILS);
	CMD3(CCC_Mask,		"r2_sun_focus",			&ps_r2_ls_flags,			R2FLAG_SUN_FOCUS);
//	CMD3(CCC_Mask,		"r2_sun_static",		&ps_r2_ls_flags,			R2FLAG_SUN_STATIC);
//	CMD3(CCC_Mask,		"r2_exp_splitscene",	&ps_r2_ls_flags,			R2FLAG_EXP_SPLIT_SCENE);
//	CMD3(CCC_Mask,		"r2_exp_donttest_uns",	&ps_r2_ls_flags,			R2FLAG_EXP_DONT_TEST_UNSHADOWED);
	CMD3(CCC_Mask,		"r2_exp_donttest_shad",	&ps_r2_ls_flags,			R2FLAG_EXP_DONT_TEST_SHADOWED);
	
	CMD3(CCC_Mask,		"r2_sun_tsm",			&ps_r2_ls_flags,			R2FLAG_SUN_TSM	);
	CMD4(CCC_Float,		"r2_sun_tsm_proj",		&ps_r2_sun_tsm_projection,	.001f,	0.8f	);
	CMD4(CCC_Float,		"r2_sun_tsm_bias",		&ps_r2_sun_tsm_bias,		-0.5,	+0.5	);
	CMD4(CCC_Float,		"r2_sun_near",			&ps_r2_sun_near,			1.f,	50.f	);

	CMD4(CCC_Float,		"r2_sun_far",			&OLES_SUN_LIMIT_27_01_07,	51.f,	180.f	);
	CMD4(CCC_Float,		"r2_sun_near_border",	&ps_r2_sun_near_border,		.5f,	1.0f	);
	CMD4(CCC_Float,		"r2_sun_depth_far_scale",&ps_r2_sun_depth_far_scale,0.5,	1.5		);
	CMD4(CCC_Float,		"r2_sun_depth_far_bias",&ps_r2_sun_depth_far_bias,	-0.5,	+0.5	);
	CMD4(CCC_Float,		"r2_sun_depth_near_scale",&ps_r2_sun_depth_near_scale,0.5,	1.5		);
	CMD4(CCC_Float,		"r2_sun_depth_near_bias",&ps_r2_sun_depth_near_bias,-0.5,	+0.5	);
	CMD4(CCC_Float,		"r2_sun_lumscale",		&ps_r2_sun_lumscale,		-1.0,	+3.0	);
	CMD4(CCC_Float,		"r2_sun_lumscale_hemi",	&ps_r2_sun_lumscale_hemi,	0.0,	+3.0	);
	CMD4(CCC_Float,		"r2_sun_lumscale_amb",	&ps_r2_sun_lumscale_amb,	0.0,	+3.0	);

	CMD3(CCC_Mask,		"r2_aa",				&ps_r2_ls_flags,			R2FLAG_AA);
	CMD4(CCC_Float,		"r2_aa_kernel",			&ps_r2_aa_kernel,			0.3f,	0.7f	);
	//CMD3(CCC_Mask, "r2_mblur_enable", &ps_r2_ls_flags, R2FLAG_MBLUR);
	CMD4(CCC_Float, "r2_mblur", &ps_r2_mblur, 0.0f, 1.5f);

	CMD3(CCC_Mask,		"r2_gi",				&ps_r2_ls_flags,			R2FLAG_GI);
	CMD4(CCC_Float,		"r2_gi_clip",			&ps_r2_GI_clip,				EPS,	0.1f	);
	CMD4(CCC_Integer,	"r2_gi_depth",			&ps_r2_GI_depth,			1,		5		);
	CMD4(CCC_Integer,	"r2_gi_photons",		&ps_r2_GI_photons,			8,		256		);
	CMD4(CCC_Float,		"r2_gi_refl",			&ps_r2_GI_refl,				EPS_L,	0.99f	);

	CMD4(CCC_Integer,	"r2_wait_sleep",		&ps_r2_wait_sleep,			0,		1		);

#ifndef MASTER_GOLD
	CMD4(CCC_Integer,	"r2_dhemi_count",		&ps_r2_dhemi_count,			4,		25		);
	CMD4(CCC_Float,		"r2_dhemi_sky_scale",	&ps_r2_dhemi_sky_scale,		0.0f,	100.f	);
	CMD4(CCC_Float,		"r2_dhemi_light_scale",	&ps_r2_dhemi_light_scale,	0,		100.f	);
	CMD4(CCC_Float,		"r2_dhemi_light_flow",	&ps_r2_dhemi_light_flow,	0,		1.f	);
	CMD4(CCC_Float,		"r2_dhemi_smooth",		&ps_r2_lt_smooth,			0.f,	10.f	);
	CMD3(CCC_Mask,		"rs_hom_depth_draw",	&ps_r2_ls_flags_ext,		R_FLAGEXT_HOM_DEPTH_DRAW);
	CMD3(CCC_Mask,		"r2_shadow_cascede_zcul",&ps_r2_ls_flags_ext,		R2FLAGEXT_SUN_ZCULLING);
	CMD3(CCC_Mask,		"r2_shadow_cascede_old", &ps_r2_ls_flags_ext,		R2FLAGEXT_SUN_OLD);
#endif // DEBUG


	CMD4(CCC_Float,		"r2_ls_depth_scale",	&ps_r2_ls_depth_scale,		0.5,	1.5		);
	CMD4(CCC_Float,		"r2_ls_depth_bias",		&ps_r2_ls_depth_bias,		-0.5,	+0.5	);

	CMD4(CCC_Float,		"r2_parallax_h",		&ps_r2_df_parallax_h,		.0f,	.5f		);
//	CMD4(CCC_Float,		"r2_parallax_range",	&ps_r2_df_parallax_range,	5.0f,	175.0f	);

	CMD4(CCC_Float,		"r2_slight_fade",		&ps_r2_slight_fade,			.2f,	1.f		);

	tw_min.set			(0,0,0);	tw_max.set	(1,1,1);
	CMD4(CCC_Vector3,	"r2_aa_break",			&ps_r2_aa_barier,			tw_min, tw_max	);

	tw_min.set			(0,0,0);	tw_max.set	(1,1,1);
	CMD4(CCC_Vector3,	"r2_aa_weight",			&ps_r2_aa_weight,			tw_min, tw_max	);

	//	Igor: Depth of field
	tw_min.set			(-10000,-10000,0);	tw_max.set	(10000,10000,10000);
	CMD4( CCC_Dof,		"r2_dof",		&ps_r2_dof, tw_min, tw_max);
	CMD4( CCC_DofNear,	"r2_dof_near",	&ps_r2_dof.x, tw_min.x, tw_max.x);
	CMD4( CCC_DofFocus,	"r2_dof_focus", &ps_r2_dof.y, tw_min.y, tw_max.y);
	CMD4( CCC_DofFar,	"r2_dof_far",	&ps_r2_dof.z, tw_min.z, tw_max.z);

	CMD4(CCC_Float,		"r2_dof_kernel",	&ps_r2_dof_kernel_size,				.0f,	10.f);
	CMD4(CCC_Float,		"r2_dof_sky",		&ps_r2_dof_sky,						-10000.f,	10000.f);
	CMD3(CCC_Mask,		"r2_dof_enable",	&ps_r2_ls_flags,	R2FLAG_DOF);
	CMD4(CCC_Vector3,	"r__color_sight",	&sight_color, Fvector3().set(0,0,0), Fvector3().set(255,255,255));
	
//	float		ps_r2_dof_near			= 0.f;					// 0.f
//	float		ps_r2_dof_focus			= 1.4f;					// 1.4f

		//ogse sunshafts
	CMD3(CCC_Token, "r2_sunshafts_mode", &ps_sunshafts_mode, sunshafts_mode_token);
	CMD4(CCC_Float, "r2_ss_sunshafts_length", &ps_r2_ss_sunshafts_length, .2f, 1.5f);
	CMD4(CCC_Float, "r2_ss_sunshafts_radius", &ps_r2_ss_sunshafts_radius, .5f, 2.f);
	//end ogse sunshafts 

	// PseudoPBR
	CMD4(CCC_Float, "r3_pbr_intensity", &ps_r3_pbr_intensity, .5f, 25.f);
	CMD4(CCC_Float, "r3_pbr_roughness", &ps_r3_pbr_roughness, .0f, 1.f);
	CMD3(CCC_Mask, "r3_pbr", &ps_r3_pbr_flags, R_FLAG_PSEUDOPBR);

	CMD3(CCC_Mask,		"r2_volumetric_lights",			&ps_r2_ls_flags,			R2FLAG_VOLUMETRIC_LIGHTS);
//	CMD3(CCC_Mask,		"r2_sun_shafts",				&ps_r2_ls_flags,			R2FLAG_SUN_SHAFTS);
	CMD3(CCC_Token,		"r2_sun_shafts",				&ps_r_sun_shafts,			qsun_shafts_token);
	CMD3(CCC_SSAO_Mode,	"r2_ssao_mode",					&ps_r_ssao_mode,			qssao_mode_token);
	CMD3(CCC_Token,		"r2_ssao",						&ps_r_ssao,					qssao_token);
	CMD3(CCC_Mask,		"r2_ssao_blur",                 &ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_BLUR);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_opt_data",				&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_OPT_DATA);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_half_data",			&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_HALF_DATA);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_hbao",					&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_HBAO);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_hdao",					&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_HDAO);//Need restart
	CMD3(CCC_Mask, "r2_ssao_hbao_plus", &ps_r2_ls_flags_ext, R2FLAGEXT_HBAO_PLUS);//Need restart
	CMD3(CCC_Mask,		"r4_enable_tessellation",		&ps_r2_ls_flags_ext,		R2FLAGEXT_ENABLE_TESSELLATION);//Need restart
	CMD3(CCC_Mask,		"r4_wireframe",					&ps_r2_ls_flags_ext,		R2FLAGEXT_WIREFRAME);//Need restart
	CMD3(CCC_Mask,		"r2_steep_parallax",			&ps_r2_ls_flags,			R2FLAG_STEEP_PARALLAX);
	CMD3(CCC_Mask,		"r2_detail_bump",				&ps_r2_ls_flags,			R2FLAG_DETAIL_BUMP);

	//Static on R2+
	CMD3(CCC_Mask, "r2_use_bump", &ps_r2_static_flags, R2FLAG_USE_BUMP);//Need restart

	CMD3(CCC_Token,		"r2_sun_quality",				&ps_r_sun_quality,			qsun_quality_token);

	//Hbao+
	CMD4(CCC_Float, "r4_hbao_plus_radius", &hbao_plus_radius, 0, 10);
	CMD4(CCC_Float, "r4_hbao_plus_bias", &hbao_plus_bias, 0, 1);
	CMD4(CCC_Float, "r4_hbao_plus_power_exponent", &hbao_plus_power_exponent, 0, 10);
	CMD4(CCC_Float, "r4_hbao_plus_blur_sharp", &hbao_plus_blur_sharp, 16, 256);

	//CMD3(CCC_Mask,		"r3_msaa",						&ps_r2_ls_flags,			R3FLAG_MSAA);
	CMD3(CCC_Token,		"r3_msaa",						&ps_r3_msaa,				qmsaa_token);
	//CMD3(CCC_Mask,		"r3_msaa_hybrid",				&ps_r2_ls_flags,			R3FLAG_MSAA_HYBRID);
	//CMD3(CCC_Mask,		"r3_msaa_opt",					&ps_r2_ls_flags,			R3FLAG_MSAA_OPT);
	CMD3(CCC_Mask,		"r3_use_dx10_1",				&ps_r2_ls_flags,			(u32)R3FLAG_USE_DX10_1);
	//CMD3(CCC_Mask,		"r3_msaa_alphatest",			&ps_r2_ls_flags,			(u32)R3FLAG_MSAA_ALPHATEST);
	CMD3(CCC_Token,		"r3_msaa_alphatest",			&ps_r3_msaa_atest,			qmsaa__atest_token);
	CMD3(CCC_Token,		"r3_minmax_sm",					&ps_r3_minmax_sm,			qminmax_sm_token);
	CMD4(CCC_detail_radius, "r__detail_radius", &ps_r__detail_radius, 49, 100);
	CMD3(CCC_Mask, "r__enable_grass_shadow", &psDeviceFlags2, rsGrassShadow); //Alundaio
	CMD3(CCC_Mask, "r__no_scale_on_fade", &psDeviceFlags2, rsNoScale); //Alundaio



	//	Allow real-time fog config reload
#if	(RENDER == R_R4)
#ifdef	DEBUG
	CMD1(CCC_Fog_Reload,"r3_fog_reload");
#endif	//	DEBUG
#endif	//	(RENDER == R_R3) || (RENDER == R_R4)

	CMD3(CCC_Mask,		"r3_dynamic_wet_surfaces",		&ps_r2_ls_flags,			R3FLAG_DYN_WET_SURF);
	CMD4(CCC_Float,		"r3_dynamic_wet_surfaces_near",	&ps_r3_dyn_wet_surf_near,	10,	70		);
	CMD4(CCC_Float,		"r3_dynamic_wet_surfaces_far",	&ps_r3_dyn_wet_surf_far,	30,	100		);
	CMD4(CCC_Integer,	"r3_dynamic_wet_surfaces_sm_res",&ps_r3_dyn_wet_surf_sm_res,64,	2048	);

	// Screen Space Shaders
	CMD4(CCC_Vector4, "ssfx_grass_shadows", &ps_ssfx_grass_shadows, Fvector4().set(0, 0, 0, 0), Fvector4().set(3, 1, 100, 100));
	//CMD4(CCC_ssfx_cascades, "ssfx_shadow_cascades", &ps_ssfx_shadow_cascades, Fvector3().set(1.0f, 1.0f, 1.0f), Fvector3().set(300, 300, 300));
	CMD4(CCC_Vector4, "ssfx_grass_interactive", &ps_ssfx_grass_interactive, Fvector4().set(0, 0, 0, 0), Fvector4().set(1, 15, 5000, 1));
	CMD4(CCC_Vector4, "ssfx_int_grass_params_1", &ps_ssfx_int_grass_params_1, Fvector4().set(0, 0, 0, 0), Fvector4().set(5, 5, 5, 60));
	CMD4(CCC_Vector4, "ssfx_int_grass_params_2", &ps_ssfx_int_grass_params_2, Fvector4().set(0, 0, 0, 0), Fvector4().set(5, 20, 1, 5));
	//CMD4(CCC_Vector4, "ssfx_hud_drops_1", &ps_ssfx_hud_drops_1, Fvector4().set(0, 0, 0, 0), Fvector4().set(100000, 100, 100, 100));
	//CMD4(CCC_Vector4, "ssfx_hud_drops_2", &ps_ssfx_hud_drops_2, Fvector4().set(0, 0, 0, 0), tw2_max);
	//CMD4(CCC_Vector4, "ssfx_blood_decals", &ps_ssfx_blood_decals, Fvector4().set(0, 0, 0, 0), Fvector4().set(5, 5, 0, 0));
	CMD4(CCC_Vector4, "ssfx_rain_1", &ps_ssfx_rain_1, Fvector4().set(0, 0, 0, 0), Fvector4().set(10, 5, 5, 2));
	CMD4(CCC_Vector4, "ssfx_rain_2", &ps_ssfx_rain_2, Fvector4().set(0, 0, 0, 0), Fvector4().set(1, 10, 10, 10));
	CMD4(CCC_Vector4, "ssfx_rain_3", &ps_ssfx_rain_3, Fvector4().set(0, 0, 0, 0), Fvector4().set(1, 10, 10, 10));
	//CMD4(CCC_Vector4, "ssfx_wind_grass", &ps_ssfx_wind_grass, Fvector4().set(0.0, 0.0, 0.0, 0.0), Fvector4().set(20.0, 5.0, 5.0, 5.0));
	//CMD4(CCC_Vector4, "ssfx_wind_trees", &ps_ssfx_wind_trees, Fvector4().set(0.0, 0.0, 0.0, 0.0), Fvector4().set(20.0, 5.0, 5.0, 1.0));
	CMD4(CCC_Integer, "ssfx_ssr_quality", &ps_ssfx_ssr_quality, 0, 2);
	CMD4(CCC_Vector4, "ssfx_ssr", &ps_ssfx_ssr, Fvector4().set(1, 0, 0, 0), Fvector4().set(2, 1, 1, 1));
	CMD4(CCC_Vector4, "ssfx_ssr_2", &ps_ssfx_ssr_2, Fvector4().set(0, 0, 0, 0), Fvector4().set(2, 2, 2, 2));
	CMD4(CCC_Vector4, "ssfx_volumetric", &ps_ssfx_volumetric, Fvector4().set(0, 0, 1.0, 1.0), Fvector4().set(1.0, 5.0, 5.0, 16.0));
	//CMD4(CCC_Vector3, "ssfx_shadow_bias", &ps_ssfx_shadow_bias, Fvector3().set(0, 0, 0), Fvector3().set(1.0, 1.0, 1.0));
	CMD4(CCC_Vector4, "ssfx_terrain_quality", &ps_ssfx_terrain_quality, Fvector4().set(0, 0, 0, 0), Fvector4().set(12, 0, 0, 0));
	CMD4(CCC_Vector4, "ssfx_terrain_offset", &ps_ssfx_terrain_offset, Fvector4().set(-1, -1, -1, -1), Fvector4().set(1, 1, 1, 1));

	// Geometry optimization
	CMD4(CCC_Integer, "r__optimize_static_geom", &opt_static, 0, 2);
	//CMD4(CCC_Integer, "r__optimize_dynamic_geom", &opt_dynamic, 0, 2);
	psDeviceFlags2.set(rsOptShadowGeom, FALSE);
//	CMD3(CCC_Mask, "r__optimize_shadow_geom", &psDeviceFlags2, rsOptShadowGeom);
	CMD4(CCC_Integer, "r2_lfx", &ps_r2_lfx, 0, 1); //SFZ Lens Flares

//	CMD3(CCC_Mask,		"r2_sun_ignore_portals",		&ps_r2_ls_flags,			R2FLAG_SUN_IGNORE_PORTALS);
}


#endif
