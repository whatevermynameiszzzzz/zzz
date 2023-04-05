// ## 2023/03/14 # Start
const int TACTIC_EXP_GAIN = 7;
const int TACTIC_EXP_GAIN_KILL = 7;
const int CANNON_AOE_TARGET = 2;

const float MAX_AMBUSH_DMG = 0.4;//40% attacked troop hp
const int MAX_AMBUSH_TROOP = 10000;
const int AMBUSH_TROOP_MULTI = 6000;

const int TACTIC_CANNON = 0;
const int TACTIC_CANNONSHIP = 1;
const int TACTIC_END = 10;

class tactic{
	int id;
	string name;
	string desc;
	string req_desc;
	int energy_cost;
	int base_dmg;
	int range_min = 1;
	int range_max = 3;
	int stat = STAT_LEAD;
	float level_dmg = 0.1f;
	int troop_multi = 7000;
	int required_tekisei = WEAPON_C;
	array<int> required_tech;
	array<int> required_army;
	array<int> required_formation;
	int required_upgrade; 
	int unit_type = UNIT_TYPE_COMBAT;
	bool autotactic = false;
}

array<tactic> tactics;

namespace rr_tactic
{	
	const bool RR_AMBUSH = true;

	class Main
	{
		float _statEFF = 0.1f;

		Main()
		{
			init_tactic();
			
			pk::bind(111, pk::trigger111_t(onTurnStart));
			pk::bind(172, pk::trigger172_t(onUnitPositionChange));
			
			//ambush
			if(RR_AMBUSH){
				pk::menu_item ambush_menu;
				ambush_menu.menu = 115;
				ambush_menu.init = pk::unit_menu_item_init_t(init);
				ambush_menu.is_visible = cast<pk::menu_item_is_visible_t@>(function() { return true; });
				ambush_menu.is_enabled = pk::menu_item_is_enabled_t(isEnabled_ambush);
				ambush_menu.get_text = cast<pk::menu_item_get_text_t@>(function() { return pk::encode("Phち binh (0)"); });
				ambush_menu.get_desc = pk::menu_item_get_desc_t(getDesc_ambush);
				ambush_menu.handler = pk::unit_menu_item_handler_t(handler_rr_ambush);
				pk::add_menu_item(ambush_menu);		
			}			

			pk::menu_item tactic_menu;
			
			tactic_menu.menu = 113;
			tactic_menu.init = pk::unit_menu_item_init_t(init);
			tactic_menu.is_visible = cast<pk::menu_item_is_visible_t@>(function(){ return main.isVisible_Tactic(TACTIC_CANNON);});
			tactic_menu.is_enabled = cast<pk::menu_item_is_enabled_t@>(function(){ return main.isEnabled_Tactic(TACTIC_CANNON);});
			tactic_menu.get_text = cast<pk::menu_item_get_text_t@>(function() { return main.getText_Tactic(TACTIC_CANNON); });
			tactic_menu.get_desc = cast<pk::menu_item_get_desc_t@>(function(){ return main.getDesc_Tactic(TACTIC_CANNON); });
			tactic_menu.get_targets = cast<pk::unit_menu_item_get_targets_t@>(function() { return main.getTargets_Tactic(TACTIC_CANNON);});
			tactic_menu.handler = cast<pk::unit_menu_item_handler_t@>(function(dst_pos){ return main.handler_Tactic(TACTIC_CANNON,dst_pos);});
			pk::add_menu_item(tactic_menu);					
			
			tactic_menu.menu = 113;
			tactic_menu.init = pk::unit_menu_item_init_t(init);
			tactic_menu.is_visible = cast<pk::menu_item_is_visible_t@>(function(){ return main.isVisible_Tactic(TACTIC_CANNONSHIP);});
			tactic_menu.is_enabled = cast<pk::menu_item_is_enabled_t@>(function(){ return main.isEnabled_Tactic(TACTIC_CANNONSHIP);});
			tactic_menu.get_text = cast<pk::menu_item_get_text_t@>(function() { return main.getText_Tactic(TACTIC_CANNONSHIP); });
			tactic_menu.get_desc = cast<pk::menu_item_get_desc_t@>(function(){ return main.getDesc_Tactic(TACTIC_CANNONSHIP); });
			tactic_menu.get_targets = cast<pk::unit_menu_item_get_targets_t@>(function() { return main.getTargets_Tactic(TACTIC_CANNONSHIP);});
			tactic_menu.handler = cast<pk::unit_menu_item_handler_t@>(function(dst_pos){ return main.handler_Tactic(TACTIC_CANNONSHIP,dst_pos);});
			pk::add_menu_item(tactic_menu);			
		}
		
		void init_tactic(){
			tactics.resize(TACTIC_END);
			
			tactic cannon;
			cannon.id = TACTIC_CANNON;
			cannon.name = "Fire Cannon";
			cannon.desc = "Fire Cannon in Land";
			cannon.req_desc = "Require S";
			cannon.energy_cost = 15;
			cannon.base_dmg = 2500;
			cannon.range_min = 3;
			cannon.range_max = 4;
			cannon.level_dmg = 0.1f;
			cannon.troop_multi = 7000;
			cannon.required_tekisei = WEAPON_S;
			cannon.required_tech = {TECH_FIRE_4, TECH_SIEGE_4};
			cannon.required_army = {WEAPON_FIREOX};
			cannon.required_upgrade = UPGRADE_CANNON; 
			cannon.required_formation = {FORMATION_CANNON_ID}; 
			cannon.unit_type = UNIT_TYPE_COMBAT;
			tactics.insertAt(TACTIC_CANNON, cannon);			
			
			tactic cannonship;
			cannonship.id = TACTIC_CANNONSHIP;
			cannonship.name = "Fire Cannon";
			cannonship.desc = "Fire Cannon in Sea";
			cannonship.req_desc = "Require S";
			cannonship.energy_cost = 20;
			cannonship.base_dmg = 2500;
			cannonship.range_min = 3;
			cannonship.range_max = 4;
			cannonship.level_dmg = 0.1f;
			cannonship.troop_multi = 7000;
			cannonship.required_tekisei = WEAPON_S;
			cannonship.required_tech = {TECH_FIRE_4, TECH_SIEGE_4};
			cannonship.required_army = {WEAPON_WARSHIP};
			cannonship.required_upgrade = UPGRADE_CANNON_SHIP; 
			cannonship.required_formation = {FORMATION_CAPITALSHIP_ID, FORMATION_CANNONSHIP_ID}; 
			cannonship.unit_type = UNIT_TYPE_COMBAT;
			tactics.insertAt(TACTIC_CANNONSHIP, cannonship);
		}
		
		bool enable_ambush;	
		bool enable_cannon;		
		
        pk::unit@ src_unit;
        pk::unit@ dst_unit;
        pk::point old_pos;
        pk::point src_pos_;
        pk::point dst_pos_;		
		pk::force@ src_force;
		
        void init(pk::unit@ unit, pk::point src_pos)
        {
            @src_unit = @unit;
            src_pos_ = src_pos;
			@src_force = pk::get_force(unit.get_force_id());
        }		
		
		array<pk::point_int> getTargets_Tactic(int tactic_id)
		{
			int min = tactics[tactic_id].range_min;
			int max = tactics[tactic_id].range_max;
			
			array<pk::point_int> targets;
			auto range = pk::range(src_pos_, max == 0 ? 0 : min, max);
			for (int i = 0; i < int(range.length); i++)
			{
				targets.insertLast(pk::point_int(range[i], 0));
			}
			return targets;
		}	

		array<pk::point_int> getTargets_TacticEnemyUnit(int tactic_id)
		{
			int min = tactics[tactic_id].range_min;
			int max = tactics[tactic_id].range_max;		
			
			array<pk::point_int> targets;
			auto range = pk::range(src_pos_, max == 0 ? 0 : min, max);
			for (int i = 0; i < int(range.length); i++)
			{
				pk::unit@ dst_unit = pk::get_unit(range[i]);

				if(dst_unit is null)continue;
				if (pk::is_enemy(src_unit, dst_unit)){
					targets.insertLast(pk::point_int(range[i], 0));
				}
			}
			return targets;
		}

		// Handle Tactic Start
		// ----------------------------------------		
		
		bool isVisible_Tactic(int tactic_id){
			bool check_tekisei = false;
			bool check_tech = false;
			bool check_upgrade = false;
			bool check_army = false;
			bool check_formation = false;
			
			check_tekisei = pk::get_tekisei(src_unit) >= tactics[tactic_id].required_tekisei;
			
			if(int((tactics[tactic_id].required_tech).length) > 0){
				for(int i = 0; i < int((tactics[tactic_id].required_tech).length); i++){
					if(pk::has_tech(src_force, tactics[tactic_id].required_tech[i])){
						check_tech = true;
					}else check_tech = false;
				}
			}else check_tech = true;				
			
			if(tactics[tactic_id].required_upgrade != 0){
				check_upgrade = rr_upgrade::get_upgrade(src_force, tactics[tactic_id].required_upgrade);
			}else check_upgrade = true;			
			
			if(int((tactics[tactic_id].required_army).length) > 0){
				for(int i = 0; i < int((tactics[tactic_id].required_army).length); i++){
					if(tactics[tactic_id].required_army[i] == src_unit.weapon){
						check_army = true;
						break;
					}
				}
			}else check_army = true;			
			
			if(int((tactics[tactic_id].required_formation).length) > 0){
				for(int i = 0; i < int((tactics[tactic_id].required_formation).length); i++){
					if(global_has_formation(src_unit, tactics[tactic_id].required_formation[i])){
						check_formation = true;
						break;
					}
				}
			}else check_formation = true;			
			
			return check_tekisei and check_tech and check_upgrade and check_army and check_formation;
		}		
		
		bool isEnabled_Tactic(int tactic_id){
			bool check_energy = false;
			
			check_energy = int(src_unit.energy) >= tactics[tactic_id].energy_cost;
			
			return check_energy;
		}		
		
		string getText_Tactic(int tactic_id){
			return pk::encode(tactics[tactic_id].name) + "( " + tactics[tactic_id].energy_cost +  " )";
		}		
		
		string getDesc_Tactic(int tactic_id){
			return pk::encode(tactics[tactic_id].desc);
		}		
		
		bool handler_Tactic(int tactic_id, pk::point dst_pos){
			if(tactics[tactic_id].autotactic)rr_autotactic::execute_autotactic(src_unit, dst_pos);	
			int tactic_result = 0;
			
			switch (tactic_id)
			{	
				case TACTIC_CANNON: tactic_result = cannon_fire(src_unit, dst_pos); break;  
				case TACTIC_CANNONSHIP: tactic_result = cannon_fire(src_unit, dst_pos); break;  
			}
			
			//assist

			//cost and stuff
			pk::add_energy(src_unit, -tactics[tactic_id].energy_cost);
			pk::add_stat_exp(src_unit, tactics[tactic_id].stat, TACTIC_EXP_GAIN);
			src_unit.action_done = (int(pk::option["San11Option.EnableInfiniteAction"]) != 0) ? false : true;
			if(int(pk::option["San11Option.EnableInfiniteAction"]) != 0)pk::set_action_done(src_unit, true);
			
			if(tactic_result == ATK_KILL or tactic_result == ATK_OCCUPIED){
				pk::add_stat_exp(src_unit, tactics[tactic_id].stat, TACTIC_EXP_GAIN_KILL);
			}			
		
			return (tactic_result == ATK_OK or tactic_result == ATK_KILL or tactic_result == ATK_OCCUPIED) ? true : false;
		}
		
		
		// Cannon Tactic Start
		// ----------------------------------------		
		
		int cannon_deal_dmg(pk::unit@ src_unit, pk::point dst_pos, int dmg){
			bool is_unit = false;
			bool is_city = false;
			int fire_result = 0;
			
			pk::unit@ dst_unit = pk::get_unit(dst_pos);	
			
			pk::damage_info info;
			info.src_pos = src_unit.pos;
			info.dst_pos = dst_pos;
			info.troops_damage = 1000;
			pk::unit_anim(src_unit, 16,{info});
			pk::wait(1);
			
			if(!(dst_unit is null)){
				is_unit = true;
				fire_result = deal_dmg(dst_unit,src_unit,dmg);				
				pk::stop_unit_anim();
				
				if (pk::is_alive(dst_unit) and pk::is_in_screen(dst_unit.pos)) {
					pk::create_effect(0x42, dst_unit.pos);
					pk::unit_anim(dst_unit,26,{info});	
				}				
			}else{
				pk::building@ dst_building = pk::get_building(dst_pos);
				
				if(dst_building !is null){
					is_city = (dst_building.facility == FACILITY_CITY);
					pk::create_effect(0x42, dst_building.pos);
					int def_dmg = int(dmg/1.7f);
					int troop_dmg = int(dmg/4.0f);	

					fire_result = deal_dmg(dst_building, src_unit, def_dmg, troop_dmg);
				}
			}

			//aoe
			int max_aoe_target = CANNON_AOE_TARGET;
			int count = 0;
			int aoe_dmg = int(dmg/5.0f);
			array<pk::point> ring_arr = pk::ring(dst_pos, 1);
			for (int i = 0; i < int(ring_arr.length); i++){
				pk::create_effect(0x42, ring_arr[i]);
				count++;
				
				pk::unit@ dst_unit = pk::get_unit(ring_arr[i]);
				if(!(dst_unit is null)){
					deal_dmg(dst_unit,src_unit,aoe_dmg);

					if(pk::is_alive(dst_unit)){
						is_unit = true;
						info.dst_pos = ring_arr[i];
						pk::unit_anim(dst_unit,26,{info});
					}
				}else{
					pk::building@ dst_building = pk::get_building(ring_arr[i]);
					if(dst_building !is null){
						int def_dmg = int(aoe_dmg/1.5f);
						int troop_dmg = int(aoe_dmg/3.0f);
						deal_dmg(dst_building, src_unit, def_dmg, troop_dmg);
					}
				}
				
				if(count >= max_aoe_target)break;
			}			
			
			if(is_unit){
				pk::wait(1);
				pk::stop_unit_anim();			
			}
			
			return fire_result;			
		}
		
		int cannon_fire(pk::unit @src_unit, pk::point dst_pos){
			int cannon_level = get_siege_tech_level(src_unit);
			int fire_result = 0;
			
			int chance = 2*ch::minZero(src_unit.attr.stat[STAT_LEAD] - 95)+cannon_level;			
			bool crit = pk::rand_bool(pk::min(100, chance));
			if(crit and bool(pk::core["strategy_cutin"])){
				pk::face_cutin(src_unit);
				pk::play_voice(0x37, pk::get_person(src_unit.leader));
			}

			int skill_mod = 1;
			float crit_mod = crit == true ? 1.3 : 1;
			
			float troop_mod = src_unit.troops/float(tactics[TACTIC_CANNON].troop_multi);			
			float stat_mod = (src_unit.attr.stat[STAT_LEAD] * 2 + src_unit.attr.stat[STAT_INT])/360.f;			
			float tekisei_mod = (pk::get_tekisei(src_unit) - 3) * tactics[TACTIC_CANNON].level_dmg;//only count from S1			
			float stat_and_tech_mod = stat_mod + tekisei_mod; 			
			
			float dmg_value = ((troop_mod * stat_and_tech_mod * skill_mod * crit_mod) * tactics[TACTIC_CANNON].base_dmg) + ch::randint(100,200);
			pk::trace("Raw dmg value = " + dmg_value);
		
			int dmg = int(dmg_value);

			fire_result = cannon_deal_dmg(src_unit, dst_pos, dmg);
			
			if(pk::is_in_screen(src_unit.pos)){//todo: 1 function to say them all
				pk::person@ src_leader = pk::get_person(src_unit.leader);

				switch (pk::rand(2))
				{	
					case 0: pk::say(pk::encode("See my cannon power"), src_leader); break;  
					case 1: pk::say(pk::encode("Boom!"), src_leader); break;  
				}
			}		

			return fire_result;
		}

		// Cannon Tactic End
		// ----------------------------------------		
		

			
		// Ambush Start
		// ----------------------------------------		
		
		bool isEnabled_ambush(){
			enable_ambush = true;
			
			if(src_unit.troops > MAX_AMBUSH_TROOP)enable_ambush = false;
			
            auto range = pk::range(src_pos_, 1, 3);
            for (int i = 0; i < int(range.length); i++)
            {
				pk::unit@ dst_unit = pk::get_unit(range[i]);
                pk::hex@ hex = pk::get_hex(range[i]);
				if (!pk::is_alive(dst_unit)) continue;
				
				//pk::trace("Found unit " + pk::decode(pk::get_name(pk::get_person(dst_unit.leader))));
				if (pk::is_enemy(src_unit, dst_unit)){
					enable_ambush = false;
					break;
				}				
            }
            return enable_ambush;
		}		
		
        string getDesc_ambush()
        {
            if (enable_ambush)
                return pk::encode("hち binh trong ph止 vi 2 逸. C杏 tＺ gぞ Cho⑮g/rг lui.");
            else return pk::encode("Kh＄g |Ⅰc 仁 |ｕh trong ph止 vi 3 逸. Troop < " + MAX_AMBUSH_TROOP + ".");
        }

        bool handler_rr_ambush(pk::point dst_pos)
        {
			//pk::trace("start ambush status");
			unitinfo@ uniti = @unit_ex[src_unit.get_id()];
			uniti.on_ambush = true;
			uniti.ambush_turn = 0;
			
			//for now, let's unit stay ambush
			uniti.ambush_maxturn = 20;
			
			uniti.update(src_unit.get_id());
			
			if (pk::is_in_screen(src_unit.pos)) pk::say(pk::encode("We will set ambush here."), pk::get_person(src_unit.leader), src_unit);
				
			//pk::trace("Set ambush (3 turn) for unit " + src_unit.get_id() + " is " + uniti.on_ambush);
			src_unit.action_done = (int(pk::option["San11Option.EnableInfiniteAction"]) != 0) ? false : true;			
				
			return true;
        }

		// Ambush End
		// ----------------------------------------		


		// Combat/Upgrade/Formation/Etc Effect Start
		// ----------------------------------------		

		void onTurnStart(pk::force@ force)
		{	
			pk::trace("Turn start for force " + force.get_id());
			
			bool change_atkdef;
			//my effect for unit
			for (int i = 0; i < 部队_末; i++){
				pk::unit@ tunit = pk::get_unit(i);
				if(!pk::is_alive(tunit))continue;
				if(force.get_id() != tunit.get_force_id())continue;
				
				if(rr_upgrade::is_upgraded(force, UPGRADE_PROPAGANDA)){
					pk::trace("Propaganda effect");
					rr_upgrade::upgrade_propaganda_effect(tunit);
				}
				
				unitinfo@ uniti = @unit_ex[i];
				
				uniti.calculate_tactic = false;
				change_atkdef = false;
				
				//buff/debuff fade way
				if(RR_DEBUG){
					pk::trace("=========================================");
					pk::trace("On turn start - calculate atk/def mod for " + pk::decode(pk::get_name(pk::get_unit(i))) + " with current atk_mod = " + uniti.atk_mod + " and current def_mod = " + uniti.def_mod + " and current move_mod = "+uniti.move_mod);
				}
				int old_atk_mod = uniti.atk_mod;
				int old_def_mod = uniti.def_mod;
				int old_move_mod = uniti.move_mod;
				
				if(RR_DEBUG){
					if(old_atk_mod !=0){pk::trace("Change atk mod - old atk value = " + uniti.atk_mod);change_atkdef = true;}
					if(old_def_mod !=0){pk::trace("Change def_mod - old def value = " + uniti.def_mod);change_atkdef = true;}
					if(old_move_mod !=0){pk::trace("Change move_mod - old move value = " + uniti.move_mod);change_atkdef = true;}
				}
				
				if(old_atk_mod >= ATK_STEP){
					uniti.atk_mod = old_atk_mod - ATK_STEP;
				}else if((old_atk_mod < ATK_STEP) and (old_atk_mod > -ATK_STEP)){
					uniti.atk_mod = 0;
				}else if(old_atk_mod <= -ATK_STEP){
					uniti.atk_mod = old_atk_mod + ATK_STEP;
				}					
				
				if(old_def_mod >= DEF_STEP){
					uniti.def_mod = old_def_mod - DEF_STEP;
				}else if((old_def_mod < DEF_STEP) and (old_def_mod > -DEF_STEP)){
					uniti.def_mod = 0;
				}else if(old_def_mod <= -DEF_STEP){
					uniti.def_mod = old_def_mod + DEF_STEP;
				}					
				
				if(old_move_mod >= MOVE_STEP){
					uniti.move_mod = old_move_mod - MOVE_STEP;
				}else if((old_move_mod < MOVE_STEP) and (old_move_mod > -MOVE_STEP)){
					uniti.move_mod = 0;
				}else if(old_move_mod <= -MOVE_STEP){
					uniti.move_mod = old_move_mod + MOVE_STEP;
				}					
				
				//rr_autotactic::execute_autotactic(tunit);					
				
				//pk::trace("Checking unit " + i + " with ambush status = " + uniti.on_ambush);
				//@todo: rewrite + combine data with formation
				if(uniti.on_ambush == true){
					pk::trace("Unit " + i + " is on ambush at turn " + uniti.ambush_turn + " with max turn = " + uniti.ambush_maxturn);

					//if(uniti.ambush_turn < uniti.ambush_turn){
					if(uniti.ambush_turn < 100){
						uniti.ambush_turn++;							
					
						//pk::trace("Let's ignore this unit " + pk::decode(pk::get_name(pk::get_person(tunit.leader))));
						
						pk::set_order(tunit, 部队任务_护卫,-1);
						pk::run_order(tunit);
						//pk::set_action_done(tunit, true);
						
						if (pk::is_in_screen(tunit.pos)) pk::say(pk::encode("I will continue ambush"), pk::get_person(tunit.leader), tunit);
					} else {
						uniti.ambush_turn = 0;
						uniti.ambush_maxturn = 0;
						uniti.on_ambush = false;
						//pk::trace("remove ambush status");
					}
					
					//uniti.update(i);
				}
				uniti.update(i);
				
				//formation effect
				if(global_has_navy_formation(tunit, FORMATION_CAPITALSHIP_ID)){
					rr_formation::capitalship_autoheal(tunit);
				}
			}				

			//my effect for base
			pk::list<pk::building@> strongholdList = ch::get_base_list(force);
			if (strongholdList.count > 0)
			{
				for (int i = 0; i < strongholdList.count; i++)
				{
					//my edit:gold and food and weapon upgrade
					//pk::trace("Check base " + i + " of force " + pk::get_name(force) + " with force id = " + force.get_id());
					if(rr_upgrade::is_upgraded(force, UPGRADE_GOLD_PRODUCTION)){
						//pk::trace("add gold upgrade");
						pk::add_gold(strongholdList[i], ch::randint(UPGRADE_GOLD_MIN,UPGRADE_GOLD_MAX), true);
					}						
					if(rr_upgrade::is_upgraded(force, UPGRADE_FOOD_PRODUCTION)){
						//pk::trace("add food upgrade");
						pk::add_gold(strongholdList[i], ch::randint(UPGRADE_FOOD_MIN,UPGRADE_FOOD_MAX), true);
					}						
					if(rr_upgrade::is_upgraded(force, UPGRADE_WEAPON_PRODUCTION)){
						//pk::trace("add weapon upgrade");
						pk::add_weapon_amount(strongholdList[i], WEAPON_SPEAR, ch::randint(UPGRADE_WEAPON_MIN, UPGRADE_WEAPON_MAX));
						pk::add_weapon_amount(strongholdList[i], WEAPON_PIKE, ch::randint(UPGRADE_WEAPON_MIN, UPGRADE_WEAPON_MAX));
						pk::add_weapon_amount(strongholdList[i], WEAPON_CROSSBOW, ch::randint(UPGRADE_WEAPON_MIN, UPGRADE_WEAPON_MAX));
						pk::add_weapon_amount(strongholdList[i], WEAPON_CAVALRY, ch::randint(UPGRADE_HORSE_MIN, UPGRADE_HORSE_MAX));
					}
					
					//my edit:ladder dmg
					rr_formation::ladder_dmg_base(strongholdList[i]);
				}
			}
		}

		void onUnitPositionChange(pk::unit@ unit, const pk::point& in pos)
		{
			//pk::printf(pk::encode("onUnitPositionChange leader=["+unit.leader+"] pos=["+pos+"]  \n"));
			
			//if unit move, ambush status reset
			if (pk::is_alive(unit)) {
				unitinfo@ unito = @unit_ex[unit.get_id()];
				unito.on_ambush = false;
				unito.ambush_turn = 0;
				unito.ambush_maxturn = 0;
				
				unito.update(unit.get_id());
				
				//pk::trace("onUnitPositionChange leader=["+unit.leader+"] pos=["+pos+"] action done =["+unit.action_done+"] \n");
			}
			
			array<pk::point> rings;
			
			for(int i = 1; i <=2; i++){//2 squares
				rings = pk::ring(pos, i);
				//pk::trace("array length = " + int(rings.length));
				
				for (int pos_index = 0; pos_index < int(rings.length); pos_index++)
				{
					pk::unit@ dst_unit = pk::get_unit(rings[pos_index]);
					
					if (!pk::is_alive(dst_unit)) continue;
					
					unitinfo@ uniti = @unit_ex[dst_unit.get_id()];
					//pk::trace("dst_unit get_id = " + dst_unit.get_id() + " ambush status = " + uniti.on_ambush);
					
					if (pk::is_enemy(unit, dst_unit) && (uniti.on_ambush == true)){
				
						//pk::trace("Ambush triggered by unit " + dst_unit.get_id());
						if (pk::is_in_screen(dst_unit.pos)) {
							pk::say("I activate my trap card", pk::get_person(dst_unit.leader), dst_unit);
							pk::wait(100);
							pk::create_effect(0x4b, dst_unit.pos);
							pk::wait(450);	
							pk::play_se(0x71, pos);
							pk::create_effect(0x24, pos);
							pk::wait(350);
							MessageAttackedAmbush(unit,0);	
							pk::wait(250);							
						}
						
						//deal dmg
						//more troop = more dmg
						//important: never kill enemy in ambush, so deal percent dmg only!
						float skill_mod = ch::has_skill(dst_unit, 特技_待伏) ? 0.23 : 0.15;//skill ambush
						float dmg_mod = pk::max(MAX_AMBUSH_DMG, skill_mod*my_StatEFF(pk::get_person(dst_unit.leader))*(dst_unit.troops/AMBUSH_TROOP_MULTI))+(pk::rand(10)/100);
						int dmg = int(dmg_mod*unit.troops);
						deal_dmg(unit,dst_unit,dmg,true);
						
						//deal effect
						if (pk::is_alive(unit)) {
							if (!ch::has_skill(unit, 特技_治军) and !ch::has_skill(unit, skill_hoachieu)){
								int rate = my_cal_yibing_rate(unit, dst_unit);
								int time_effect;
								
								if(unit.attr.stat[部队能力_智力] >= 90){
									time_effect = pk::rand(2)+1;
								}else if(unit.attr.stat[部队能力_智力] >= 75){
									time_effect = pk::rand(2)+2;
								}else{
									time_effect = pk::rand(2)+3;
								}
								
								if (pk::rand_bool(rate)){
									int effect_type = pk::rand(2);
									if(effect_type == 0)pk::set_status(unit, dst_unit, 部队状态_伪报, time_effect,true);
									if(effect_type == 1)pk::set_status(unit, dst_unit, 部队状态_混乱, time_effect,true);
									MessageAttackedAmbush(unit,effect_type + 1);	
									pk::wait(250);	
								}
							} 
						}

						uniti.on_ambush = false;
						uniti.ambush_turn = 0;
						uniti.ambush_maxturn = 0;
						
						uniti.update(dst_unit.get_id());
						//pk::trace("Ambush status of unit source " + dst_unit.get_id() + " is " + uniti.on_ambush);
					}

				}
			}
		}
		
		// General Combat Function
		// ----------------------------------------		

		//my edit cal_yibing_rate - copy from yibing file
		int my_cal_yibing_rate(pk::unit@attacker, pk::unit@target_unit)
		{
			int rate = 疑兵基础概率;
			
			rate += pk::get_person(attacker.leader).character == 性格_莽撞 ? 5 : 0;
			
			rate += pk::get_building(pk::get_building_id(target_unit.pos)).get_force_id() == target_unit.get_force_id() ? 5 : 0;
			
			pk::hex@dst_hex = pk::get_hex(target_unit.pos);
			int terrain_id = dst_hex.terrain;
			rate += (terrain_id == 地形_湿地 or terrain_id == 地形_森 or terrain_id == 地形_栈道) ? 10 : 0;
			int dst_province_id = pk::get_province_id(target_unit.pos);
			rate += (dst_province_id == 州_益州 or dst_province_id == 州_南中) ? 5 : 0;
			
			rate -= pk::max(0, (attacker.attr.stat[部队能力_智力] - 减少疑兵概率智力下限));
			rate += pk::max(0, (target_unit.attr.stat[部队能力_智力] - 增加疑兵概率智力下限));
			
			if(ch::has_skill(target_unit, 特技_待伏))rate += 15;
			return pk::min(100,rate);
		}		
		
		void MessageAttackedAmbush(pk::unit@ u0, int type)
		{
			pk::person@ p0 = pk::get_person(u0.leader);
			if (!pk::is_alive(p0))return;
			
			if(type == 0){
				switch (pk::rand(3))
				{
				case 0: pk::say(pk::encode("We got ambushed!"), p0); break;
				case 1: pk::say(pk::encode("What is that??"), p0); break;
				case 2: pk::say(pk::encode("uh ah ah....1267"), p0); break;
				}
			}
			if(type == 1){
				switch (pk::rand(2))
				{
				case 0: pk::say(pk::encode("Stop retreating!"), p0); break;
				case 1: pk::say(pk::encode("Fake news!"), p0); break;
				}
			}
			if(type == 2){
				switch (pk::rand(2))
				{
				case 0: pk::say(pk::encode("We got confused!"), p0); break;
				case 1: pk::say(pk::encode("Stay calm!"), p0); break;
				}
			}
		}
		
		float my_StatEFF(pk::person@ p0)
		{

			if (!pk::is_alive(p0))return 0.1f;
			float eff = (
				p0.stat[武将能力_统率] +
				p0.stat[武将能力_武力] 
				) / float(2 * 120.f);

			//pk::printf(pk::encode("StatEFF eff=["+eff+"]\n"));

			return eff;
		}		
		
		bool kill_unit(pk::unit@ dst_unit, pk::unit@ src_unit){
			if (pk::is_in_screen(dst_unit.pos)) pk::create_effect(0x23,dst_unit.pos);
			pk::kill(dst_unit, src_unit, true);
			
			return true;
		}
		
		int deal_dmg(pk::unit@ attacked_unit, pk::unit@ attacker_unit, int dmg, bool do_not_kill = false){
			if(pk::is_alive(attacked_unit) and pk::is_alive(attacker_unit)){
				dmg = int(batt::get_troops_final_damage(dmg, attacker_unit, attacked_unit));
				if(RR_DEBUG)pk::trace("Deal dmg " + dmg);
				
				if(do_not_kill){
					if(dmg >= int(attacked_unit.troops))dmg = int(attacked_unit.troops - 100);
				}
				pk::add_troops(attacked_unit, -dmg, true);

				if(attacked_unit.troops <=0){
					if(RR_DEBUG)pk::trace("Kill!");
					kill_unit(attacked_unit,attacker_unit);
					return ATK_KILL;//kill
				}
			}else return ATK_FAIL;//fail to deal dmg
			
			return ATK_OK;//deal dmg ok
		}		
		
		int deal_dmg(pk::building@ attacked_building, pk::unit@ attacker_unit, int def_dmg, int troop_dmg = 0, bool do_not_kill = false){
			if(pk::is_alive(attacked_building) and pk::is_alive(attacker_unit)){
				int hp_damage = int(batt::get_hp_final_damage(def_dmg, attacker_unit, attacked_building));					
				
				if(RR_DEBUG)pk::trace("Deal def dmg " + hp_damage);
				if(RR_DEBUG)pk::trace("Deal troop dmg " + troop_dmg);	
				
				pk::add_hp(attacked_building, -hp_damage, true);
				
				if(attacked_building.facility == FACILITY_CITY and troop_dmg > 0){
					ch::add_troops(attacked_building, -troop_dmg, true);
				}
				
				if(attacked_building.hp <= 0 and attacked_building.facility != FACILITY_CITY and !do_not_kill){
					//check if named pos
					bool is_named_pos = false;
					
					if (地名设施.find(attacked_building.facility) >= 0){
						for (int i = 0; i < int(特殊地名设施.length); i++)
						{
							pk::point pos0;
							pos0.x = 特殊地名设施[i][1];
							pos0.y = 特殊地名设施[i][2];
							if (pos0 == attacked_building.pos)
							{
								//pk::trace(pk::format("pos 3,building_id:{}",building_id ));
								pk::set_district(attacked_building, pk::get_district(attacker_unit.get_district_id()), /*occupied*/0);
								ch::reset_spec(i);
								attacked_building.hp = pk::get_max_hp(attacked_building);
								is_named_pos = true;
								if(RR_DEBUG)pk::trace("Found named pos, change ownership!");
								return ATK_OCCUPIED;
							}
						}
					}

					if(!is_named_pos){
						if(RR_DEBUG)pk::trace("Kill building!");
						pk::kill(attacked_building);
						return ATK_KILL;
					}
				}
			}else return ATK_FAIL;//fail to deal dmg
			
			return ATK_OK;//deal dmg ok
		}

		
	} // end - Main
	Main main;
} // end - namespace

// Global Helper
// @todo: move to global file
// ----------------------------------------		
int get_cavalry_tech_level(pk::unit@ unit){
	pk::force@ force = pk::get_force(unit.get_force_id());

	return global_get_cavalry_tech_level(force);
}	

int get_pike_tech_level(pk::unit@ unit){
	pk::force@ force = pk::get_force(unit.get_force_id());

	return global_get_pike_tech_level(force);
}

int get_bow_tech_level(pk::unit@ unit){
	pk::force@ force = pk::get_force(unit.get_force_id());

	return global_get_bow_tech_level(force);
}	

int get_spear_tech_level(pk::unit@ unit){
	pk::force@ force = pk::get_force(unit.get_force_id());

	return global_get_spear_tech_level(force);
}

int get_siege_tech_level(pk::unit@ unit){
	pk::force@ force = pk::get_force(unit.get_force_id());

	return global_get_siege_tech_level(force);
}

int global_get_cavalry_tech_level(pk::force@ force){
	int level = 0;
	if(pk::has_tech(force, TECH_CAVALRY_1))level = 1;
	if(pk::has_tech(force, TECH_CAVALRY_2))level = 2;
	if(pk::has_tech(force, TECH_CAVALRY_3))level = 3;
	if(pk::has_tech(force, TECH_CAVALRY_4))level = 4;
	
	return level;
}

int global_get_pike_tech_level(pk::force@ force){
	int level = 0;
	if(pk::has_tech(force, TECH_PIKE_1))level = 1;
	if(pk::has_tech(force, TECH_PIKE_2))level = 2;
	if(pk::has_tech(force, TECH_PIKE_3))level = 3;
	if(pk::has_tech(force, TECH_PIKE_4))level = 4;

	return level;
}

int global_get_bow_tech_level(pk::force@ force){
	int level = 0;
	if(pk::has_tech(force, TECH_BOW_1))level = 1;
	if(pk::has_tech(force, TECH_BOW_2))level = 2;
	if(pk::has_tech(force, TECH_BOW_3))level = 3;
	if(pk::has_tech(force, TECH_BOW_4))level = 4;

	return level;	
}

int global_get_spear_tech_level(pk::force@ force){
	int level = 0;
	if(pk::has_tech(force, TECH_SPEAR_1))level = 1;
	if(pk::has_tech(force, TECH_SPEAR_2))level = 2;
	if(pk::has_tech(force, TECH_SPEAR_3))level = 3;
	if(pk::has_tech(force, TECH_SPEAR_4))level = 4;	
	
	return level;
}

int global_get_siege_tech_level(pk::force@ force){
	int level = 0;
	if(pk::has_tech(force, TECH_SIEGE_1))level = 1;
	if(pk::has_tech(force, TECH_SIEGE_2))level = 2;
	if(pk::has_tech(force, TECH_SIEGE_3))level = 3;
	if(pk::has_tech(force, TECH_SIEGE_4))level = 4;	
	
	return level;
}

int global_get_troop_tech_level(pk::force@ force){
	int level = 0;
	if(pk::has_tech(force, TECH_TROOP_1))level = 1;
	if(pk::has_tech(force, TECH_TROOP_2))level = 2;
	if(pk::has_tech(force, TECH_TROOP_3))level = 3;
	if(pk::has_tech(force, TECH_TROOP_4))level = 4;	
	
	return level;
}