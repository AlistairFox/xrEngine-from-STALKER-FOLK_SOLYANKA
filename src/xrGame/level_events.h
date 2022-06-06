#pragma once
 
class level_events : public ISheduled
{ 
public:
	
   
	level_events();
	~level_events();

	// Унаследовано через ISheduled
	virtual float shedule_Scale() override;
	virtual bool shedule_Needed() override;
	virtual shared_str shedule_clsid() override;

	virtual void shedule_Update(u32 dt);
};

