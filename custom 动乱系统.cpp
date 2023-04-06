// ## 2022/09/24 # 123 # 分化君主官职，同意分化、左迁与原势力同盟 ##
// ## 2022/09/23 # 123 # 狂士 ##
// ## 2022/09/23 # 123 # 君主之死玩家处理 ##
// ## 2022/09/22 # 123 # 君主之死Ai处理 ##
// ## 2022/09/19 # 123 # 君主之死 ##
// ## 2022/09/19 # 123 # 汉帝出逃新机制 ##
// ## 2022/09/15 # 123 # 四夷来朝，四夷更替 ##
// ## 2022/09/14 # 123 # 兼容五贼外交 ##
// ## 2022/09/13 # 123 # 节日 ##
// ## 2022/09/10 # 123 # 左迁、蜂起后君主独立、分化同意转让封地 ##
// ## 2022/09/08 # 黑店小小二 # 封地算法 ##
// ## 2022/09/05 # 123 # 左迁 ##
// ## 2022/09/03 # 123 # 独立 ##
// ## 2022/09/03 # 123 # 触发改为每年 ##
// ## 2022/08/23 # 123 # 防事件军师补丁 ##
// ## 2022/08/23 # 123 # 兼容汉官 ##
// ## 2022/08/23 # 123 # 分化系统 ##
// ## 2022/08/21 # 123 # 黄巾军被收编 ##
// ## 2022/08/21 # 123 # 蜂起汉帝被废会重置寿命，蜂起君主被篡位可能被收义子 ##
// ## 2022/08/19 # 123 # 新分化 ##
// ## 2022/08/19 # 123 # 新蜂起 ##
// ## 2022/08/19 # 123 # 新起义 ##
// ## 2022/08/18 # 龙马 # 分化 ##
// ## 2022/03/30 # 123 # Bug修复 ##
// ## 2022/03/11 # 123 # Ai起义逻辑完善 ##
// ## 2022/02/27 # 123 # 爵位号设定为10 ##
// ## 2022/02/20 # 123 # 新定义避免BUG ##
// ## 2022/01/27 # 123 # 起义事件 ##
/***CUSTOM
@name 动乱系统
@class
@author 123、龙马
@contact 
@version 2.8.4
@date 2022/09/24
@desc 一切混乱的源头。
@notice
***/

namespace 动乱
{
	// ================ CUSTOMIZE ================
	const int KEY = pk::hash("动乱");
	const array<int> 四异首领 = {武将_羌头目,武将_山越头目,武将_乌丸头目,武将_南蛮头目};
	const bool 测试接口 = false;
	// ===========================================
	class Main
	{
		pk::person@ emperor;
		pk::building@ 镜头;
		//君主之死场景人物
		pk::person@ 病君;
		pk::person@ 华佗 = pk::get_person(武将_华佗);
		pk::person@ 于吉 = pk::get_person(武将_于吉);
		pk::person@ 祢衡 = pk::get_person(武将_祢衡);
		//四夷来朝场景人物
		pk::person@ 权臣;
		pk::person@ 军师;
		pk::person@ 羌君 = pk::get_person(武将_羌头目);
		pk::person@ 越君 = pk::get_person(武将_山越头目);
		pk::person@ 乌君 = pk::get_person(武将_乌丸头目);
		pk::person@ 蛮君 = pk::get_person(武将_南蛮头目);
		int 拥帝势力编号 = -1;
		
		Main()
		{			
			pk::bind(110, pk::trigger110_t(动乱_每年));
			pk::bind(109, pk::trigger109_t(动乱_每季));
			pk::bind(107, pk::trigger107_t(测试_每日));
		}
		pk::random randint(pk::rand());
		void 移动镜头()
		{
			pk::move_screen(镜头.get_pos());
		}
		void 动乱_每年()
		{
			AI_起义();//起义（20%）：势力加入黄巾
			AI_蜂起();//蜂起（10%）：皇帝造反
			AI_分化();//分化（15%）：势力分化
			AI_节日();//节日(100%)：奖赏500P
			//AI_君死();//君死（约20%）：君主死亡 my edit: disable...
			AI_来朝();//四夷来朝(100%):封授爵位
			AI_蛮易();//四夷更替：四夷君主更替
			AI_狂士();//狂士（10%）：太守为狂士辱骂
			return;
		}
		void 动乱_每季()
		{
			AI_独立();//独立（20%）：在野独立
			AI_左迁();//左迁（15%）：势力武将远调
		}
		void 测试_每日()
		{
			//AI_勒索();
			return;
		}
		void 五贼外交(pk::force@ force_t,int 羌,int 越,int 乌,int 蛮,int 贼)
		{
			pk::add_relations(force_t.get_id(),force_t.get_id(),100);
			pk::add_relations(force_t.get_id(),pk::get_person(武将_羌头目).get_force_id(),羌);
			pk::add_relations(force_t.get_id(),pk::get_person(武将_山越头目).get_force_id(),越);
			pk::add_relations(force_t.get_id(),pk::get_person(武将_乌丸头目).get_force_id(),乌);
			pk::add_relations(force_t.get_id(),pk::get_person(武将_南蛮头目).get_force_id(),蛮);
			pk::add_relations(force_t.get_id(),pk::get_person(武将_贼将).get_force_id(),贼);
		}
		//获得城市封地
		void 获取空城封地(pk::city@ city,pk::force@ force_t)
		{
			for (int i = 0; i < 特殊地名设施.length; ++i)
			{
				pk::point pos0 = pk::point(特殊地名设施[i][1], 特殊地名设施[i][2]);
				int city_id = pk::get_city_id(pos0);
        		if (city.get_id() != city_id) continue;
        		pk::building@ building0 = pk::get_building(pos0);
        		if (building0 is null) continue;
        		if (building0.get_force_id() != -1) continue;
				building0.init_force = force_t.get_id();
			}
		}
		//失去/转让城市封地
		void 失去城市封地(pk::city@ city,pk::force@ force_t,pk::force@ force_u = null)
		{
			for (int i = 0; i < 特殊地名设施.length; ++i)
			{
				pk::point pos0 = pk::point(特殊地名设施[i][1], 特殊地名设施[i][2]);
				int city_id = pk::get_city_id(pos0);
        		if (city.get_id() != city_id) continue;
        		pk::building@ building0 = pk::get_building(pos0);
        		if (building0 is null) continue;
        		if (building0.get_force_id() != force_t.get_id()) continue;
				if(force_u is null) building0.init_force = -1;
				else building0.init_force =  force_u.get_id();
			}
		}
		//起义：20%
		void AI_起义()
		{
			int num=pk::rand(100);
			if(num>10 and num<90) return;
			auto force_list = pk::list_to_array(pk::get_force_list());
			int nlength = pk::rand(force_list.length);//Roll一个随机数字
			pk::force@ force_t = force_list[nlength];
			pk::person@ kunshu_t = pk::get_person(force_t.kunshu);
			pk::building@ kunshu_building = pk::get_building(kunshu_t.service);
			@emperor = pk::get_person(pk::get_scenario().emperor);
			if(force_t.kokugou == 5 and force_t.kunshu != 武将_张角 and force_t.kunshu != 武将_张鲁)//黄巾军被收编
			{				
				pk::message_box(pk::encode("Hoすg thi⑤, chung quy ｃ vong!.6960"), kunshu_t);
				pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x khД ng㌻a th字 b竺, ㈹ triヰ |ｐh tr⑦ 斯 h腫 nh字..6961", pk::decode(pk::get_name(force_t)))));
				kunshu_t.rank = 80;
				kunshu_t.update();
				force_t.title = 9;//爵位修正
				force_t.kokugou = -1;//国号删除
				force_t.update();
				return;
			}
			else if(force_t.kokugou == 5 and (force_t.kunshu == 武将_张角 or force_t.kunshu == 武将_张鲁))//黄巾军造反
			{
				pk::message_box(pk::encode("Trｉ xanh 迦 ch社, khざ vすg |〒ng l┃, tuо嗣 gi斯, thi⑤ 刺 |竺 盛t!.6962"), kunshu_t);
				pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x khД x祉ng ph旨 loが..6963", pk::decode(pk::get_name(force_t)))));
				force_t.title = 10;//爵位修正
				五贼外交(force_t,0,0,0,0,10);
				force_t.update();
				if(kunshu_t.service < 41)//君主在城市
				{
					pk::city@ kunshu_city = pk::building_to_city(pk::get_building(kunshu_t.service));
					ch::add_troops(kunshu_city,10000, true);
				}
				return;
			}
			else if(force_t.is_player())//玩家选中
			{
				if (pk::choose({ pk::encode("郡"), pk::encode("Kh＄g") }, pk::encode("C杏muズ p△t 暇ng khД ng㌻a Khざ Vすg?\n.6964"), kunshu_t) == 1)
					return;
				pk::message_box(pk::encode("D■ chПg l┬ than, kh＄g bｖg 七y ta thすh l┃ khД ng㌻a qu■, ph旨 kh⑮g triヰ |ｐh chｑh s雌h 遂n b志!.6965"), kunshu_t);
				if(force_t.kunshu == 武将_张角 or force_t.kunshu == 武将_张梁 or force_t.kunshu == 武将_张宝)
				{
					pk::message_box(pk::encode("L守thｉ |iз p△t 暇ng khД ng㌻a!.6966"), kunshu_t);
				}
				else
				{
					if(pk::rand(10)<5)
					{				
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x gia nh┃ khざ vすg qu■, |⑫ triヰ |ｐh khД x祉ng ph旨 loが 整 tｐh b竺 嘘, qu■ c㌧ ㈹ 屡 益..6967", pk::decode(pk::get_name(force_t)))));
						pk::kill(kunshu_t);//处死剧情
						return;
					}
				}
				if(pk::is_protecting_the_emperor(force_t))//如果拥立汉帝
				{
					pk::person@ reporter = pk::get_person(武将_文官);
					pk::message_box(pk::encode("B姿 盛o c㌧ c＄g, trong thすh p△t hiご |ang muズ 持o vong h⑮ 算, 屡 t楚 n人 t㍊ n事?.6968"),reporter );
					if (pk::choose({ pk::encode("郡"), pk::encode("Kh＄g") }, pk::encode("C杏斯 c㍊ c┬ h⑮ 算?.6969"), kunshu_t) == 0)
					{
						pk::message_box(pk::encode("Hoすg thi⑤ c人a 迦 算n, b└ 仁c hoすg 算 lド uy hiｚ c人 h紫!.6970"), kunshu_t);	
					}
					else
					{
						pk::message_box(pk::encode("Trｉ xanh 迦 ch社, hoすg 算 迦 kh＄g c□ thi社 t孜 t竺!.6971"), kunshu_t);
						pk::kill(emperor, kunshu_t, kunshu_building, null, 0);
					}
				}			
				pk::message_box(pk::encode("Trｉ xanh 迦 ch社, khざ vすg |〒ng l┃, tuо嗣 gi斯, thi⑤ 刺 |竺 盛t!.6962"), kunshu_t);
				kunshu_t.rank = 80;
				kunshu_t.update();
				force_t.kokugou = 5;
				force_t.title = 10;
				五贼外交(force_t,0,0,0,0,30);
				force_t.update();
				//忠诚处理
				int kunshu_0 = kunshu_t.get_id();
				auto city_list = pk::list_to_array(pk::get_city_list());
				for (int i = 0; i < int(city_list.length); i++)
				{
					pk::city@ city_ = city_list[i];
					pk::building@ building_ = pk::city_to_building(city_);
					pk::person@ taishu = pk::get_person(pk::get_taishu_id(building_));
					if (!building_.is_player()) continue;
					auto mibun_list = pk::list_to_array(pk::get_person_list(building_, pk::mibun_flags(身份_都督, 身份_太守, 身份_一般)));
					if (pk::is_alive(taishu))
					{
						if (0 < mibun_list.length)
						{
							for (int j = 0; j < int(mibun_list.length); j++)
							{
								pk::person@ person_0 = mibun_list[j];
								person_0.rank=80;
								if (pk::is_gikyoudai(person_0, kunshu_0) or pk::is_fuufu(person_0, kunshu_0) or pk::is_like(person_0, kunshu_0))
									continue;
								// 武将对新君主的忠诚程度的变化。
								if (person_0.disliked[0] == kunshu_t.get_id() or person_0.disliked[1] == kunshu_t.get_id()
									or person_0.disliked[2] == kunshu_t.get_id() or person_0.disliked[3] == kunshu_t.get_id()
									or person_0.disliked[4] == kunshu_t.get_id())
								{pk::add_loyalty(person_0, -50);}
								// 重视汉室变化
								if (person_0.kanshitsu == 한실_중시)
								{
									pk::add_loyalty(person_0, -15);
								}
								// 한실_보통 충성도 변화
								else if (person_0.kanshitsu == 한실_보통)
								{
									pk::add_loyalty(person_0, 0);
								}
								// 한실_무시 충성도 변화
								else if (person_0.kanshitsu == 한실_무시)
								{
									pk::add_loyalty(person_0, 15);
								}
							} // for
						} // if 
					} // if 
				} // for
				pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x gia nh┃ khざ vすg qu■, |⑫ triヰ |ｐh khД x祉ng ph旨 loが..6972", pk::decode(pk::get_name(force_t)))));
			}
			else if(kunshu_t .kanshitsu == 汉室_重视 or force_t.kunshu == 武将_曹操 or force_t.kunshu == 武将_刘备 or force_t.title == 爵位_皇帝 or force_t.title == 爵位_王)
			{
				if(force_t.title > 爵位_大将军)
				{
					force_t.title = force_t.title -1;
					force_t.update();
					pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x tr⑦ 斯 |∵ ph〒ng ti② ph止 vi khД ng㌻a, 迦 c拙u triヰ |ｐh ngｇ khen..6973", pk::decode(pk::get_name(force_t)))));
				}
				else pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x tr⑦ 斯 |∵ ph〒ng ti② ph止 vi khД ng㌻a..6974", pk::decode(pk::get_name(force_t)))));
				return;
			}
			else//Ai发动起义
			{
				pk::message_box(pk::encode("D■ chПg l┬ than, kh＄g bｖg 七y ta thすh l┃ khД ng㌻a qu■, ph旨 kh⑮g triヰ |ｐh chｑh s雌h 遂n b志!.6965"), kunshu_t);
				if(pk::rand(10)<8 or force_t.kunshu == 武将_张角 or force_t.kunshu == 武将_张梁 or force_t.kunshu == 武将_张宝)
				{
					pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x gia nh┃ khざ vすg qu■, |⑫ triヰ |ｐh khД x祉ng ph旨 loが..6972", pk::decode(pk::get_name(force_t)))));
					force_t.kokugou = 5;
					force_t.title = 10;
					五贼外交(force_t,0,0,0,0,30);
					kunshu_t.rank = 80;
					kunshu_t.update();					
				}
				else
				{
					pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x tr⑦ 斯 khД ng㌻a th字 lｇ, 迦 c拙u triヰ |ｐh tr雌h phΤ..6975", pk::decode(pk::get_name(force_t)))));
					if(force_t.title < 9)
					{
						force_t.title = force_t.title +1;	
					}
				}
				force_t.update();
				return;
			}
		}
		//皇帝换城市
		pk::list<pk::city@> selectable_city_list_for_emperor(pk::person@ emperor)
		{
			pk::city@ city = pk::get_city(emperor.service);
			pk::list<pk::city@> selectable_city_list;
			for (int i = 0; i < 6; ++i)
			{
				if(city.neighbor[i] != 城市_无) selectable_city_list.add(pk::get_city(city.neighbor[i]));
			}
			return selectable_city_list;
		}
		//蜂起：10%
		void AI_蜂起()
		{			
			int num=pk::rand(100);
			if(num>5 and num<95) return;
			auto force_list = pk::list_to_array(pk::get_force_list());
			int nlength = pk::rand(force_list.length);//Roll一个随机数字
			pk::force@ force_t = force_list[nlength];
			pk::person@ kunshu_t = pk::get_person(force_t.kunshu);
			pk::person@ gunshi = pk::get_person(force_t.gunshi);				
			pk::building@ kunshu_building = pk::get_building(kunshu_t.service);
			pk::person@ reporter = pk::get_person(武将_文官);
			@emperor = pk::get_person(pk::get_scenario().emperor);
			if(force_t.gunshi >=700 and force_t.gunshi <=762)//事件军师修复
			{
				force_t.gunshi = -1;
				force_t.update();
				return;
			}
			else if(pk::is_protecting_the_emperor(force_t))//拥立皇帝
			{
				string e1= pk::decode(pk::get_name(emperor));
				if(force_t.kunshu == 武将_董卓 or force_t.kunshu == 武将_何进 or force_t.kunshu == 武将_李傕 or force_t.kunshu == 武将_郭汜
					or force_t.kunshu == 武将_曹丕  or force_t.kunshu == 武将_司马师 or force_t.kunshu == 武将_司马昭 or force_t.kunshu == 武将_司马炎 
					or force_t.kunshu == 武将_袁绍  or force_t.kunshu == 武将_袁术)
				{					
					pk::message_box(pk::encode("B姿 盛o c㌧ c＄g, h⑮ 算 清c li⑤ l⑳ |竺 th□ hすh thｎh c㌧ c＄g, hiご t竺 迦 ㈹ trル 吸, 屡 t楚 n人 t㍊ n事?.6976"),reporter);
					pk::message_box(pk::encode("B袷刺 仕逝 t志 ph旨?.6977"), kunshu_t);
					pk::message_box(pk::encode("C㌧ c＄g trong n祉c ng→i v┥g, sao kh＄g |竺 h⑮ t姐l┃?.6978"),reporter);
					if(pk::rand(10)>1)
					{
						pk::message_box(pk::encode("Kh＄g tＺ, c㌧ c＄g nghi hすh y Do脂 ho〓 quang chi chuyご xバ....6979"),gunshi);
						pk::message_box(pk::encode("Qu■ 灼 chi ng＄, 精c vユ l┿g ta..6980"), kunshu_t);							
						emperor.sei = pk::get_person(pk::rand(670)).mei;
						emperor.birth = pk::get_year() - 7;
						emperor.death = emperor.birth + randint(50, 75);
						emperor.update();
						string e2= pk::decode(pk::get_name(emperor));
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x p㍊ \x1b[1x{}\x1b[0x 嫉 t染 d■, k△c tuy⑲ t＄g th字 嫉 算, t＄ λo \x1b[1x{}\x1b[0x, n殊 quyレ t㍊ doanh thi⑤, c人 h紫 kh＄g d氏 gh衣m└..6686",pk::decode(pk::get_name(kunshu_t)),e1,e2)));
						if(force_t.title > 爵位_王)
						{
							force_t.title = force_t.title -1;
							force_t.update();
						}
					}
					else
					{
						pk::message_box(pk::encode("Nhブg, nay 仁 |iン lすh hiご ra....6981"),gunshi);
						pk::message_box(pk::encode("Qu■ 灼 chi ng＄, 精c vユ l┿g ta..6980"), kunshu_t);	
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x 清c p㍊ \x1b[1x{}\x1b[0x xブg 算, ㈹ |竺 th□ tru 性t..6982",pk::decode(pk::get_name(kunshu_t)),e1)));
						pk::kill(kunshu_t);
					}
				}
				else
				{
					pk::message_box(pk::encode(pk::format("\x1b[1x{}\x1b[0x phi d〒ng 〒ng ngがh, nГ |ぞ phi l⑬ 芝 nГ....6983",pk::decode(pk::get_name(kunshu_t)))),emperor);
					pk::list<pk::city@> selectable_city_list = selectable_city_list_for_emperor(emperor);
					pk::city@ 出逃城市 = selectable_city_list[pk::rand(int(selectable_city_list.count))];
					if( pk::city_to_building(出逃城市).get_force_id() == force_t.get_id())
					{
						emperor.sei = pk::get_person(pk::rand(670)).mei;
						emperor.birth = pk::get_year() - 7;
						emperor.death = emperor.birth + randint(50, 75);
						emperor.update();
						string e2= pk::decode(pk::get_name(emperor));
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x p㍊ \x1b[1x{}\x1b[0x 嫉 t染 d■, k△c tuy⑲ t＄g th字 嫉 算, t＄ λo \x1b[1x{}\x1b[0x, n殊 quyレ t㍊ doanh thi⑤, c人 h紫 kh＄g d氏 gh衣m└..6686",pk::decode(pk::get_name(kunshu_t)),e1,e2)));
						if(force_t.title > 爵位_王)
						{
							force_t.title = force_t.title -1;
							force_t.update();
						}
					}
					else
					{
						int building_id = pk::city_to_building(出逃城市).get_id();
						pk::set_service(emperor, building_id);
						emperor.location = building_id;
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x phi d〒ng 〒ng ngがh, \x1b[1x{}\x1b[0x b字 kham c拙u |パg trズ |i..6984",pk::decode(pk::get_name(kunshu_t)),e1)));
					}
				}
				return;
			}
			else if(gunshi is null or force_t.kunshu == 武将_曹操 or force_t.kunshu == 武将_曹丕 or force_t.kunshu == 武将_刘邦 or force_t.kunshu == 武将_刘备
				or force_t.kunshu == 武将_嬴政 or force_t.kunshu == 武将_项籍 or force_t.kunshu == 武将_司马炎)//雄主和无军师免疫
			{
				return;
			}
			else //非拥立皇帝势力
			{
				pk::message_box(pk::encode(pk::format("\x1b[1x{}\x1b[0x 暇c 遂i quyレ to, coi ta 嫉 con r⑫....6985",pk::decode(pk::get_name(gunshi)))),kunshu_t);
				pk::message_box(pk::encode("C㌧ c＄g 仕逝 t志 ph旨?.6986"), gunshi);
				//获取宗室列表
				pk::list<pk::person@> list_t = pk::get_person_list(force_t, pk::mibun_flags(身份_都督, 身份_太守, 身份_一般));
				list_t.sort(function(a, b)
            				{
					return a.birth > b.birth;// 按年龄小排序
				});
				pk::list<pk::person@> list_ket;//宗室
				for(int i=0;i<list_t.count;i++)
				{
					if(list_t[i].ketsuen == kunshu_t.ketsuen and list_t[i].get_id() != force_t.gunshi) 
						list_ket.add(list_t[i]);
				}
				int a=pk::rand(2);
				if(force_t.kunshu == 武将_孙权) a=1;//孙权必分裂
				if(pk::get_city_count(force_t)==1 and a==1) a=0;//独城势力
				//君主造反
				if(a==0)//伊尹霍光
				{
					kunshu_t.mibun = 身份_一般;
					kunshu_t.rank = 80;
					kunshu_t.update();
					if(list_ket.count == 0)//伊尹
					{
						//宗室已绝						
						force_t.gunshi= -1;
						gunshi.mibun = 身份_君主;
						gunshi.update();
						force_t.kunshu = gunshi.get_id();
						force_t.update();
						if(kunshu_t.get_id() == 武将_曹髦)//曹髦必死
						{
							pk::kill(kunshu_t);//处死剧情
						}
						pk::message_box(pk::encode("C㌧ c＄g huy社 m⑳h 迦 tuyヱ, thロh |竺 nh■ 垂m chｑh |竺 ｄ!.6987"),reporter);
						if(kunshu_t.birth > gunshi.birth and force_t.kunshu != 武将_曹髦)//伊尹放太甲
						{
							pk::message_box(pk::encode(pk::format("Qu■ c㌧ b字 ∟o, chПg th□ 枝g l┃ ta \x1b[1x{}\x1b[0x 嫉 t■ qu■, |Пg ｃ b字 |〓 煙....6988",pk::decode(pk::get_name(gunshi)))),gunshi);
							pk::message_box(pk::encode(pk::format("Ta \x1b[1x{}\x1b[0x 清c lΗ theo y Do脂 ph┷g q蔀 gi斯, trち xu字 qu■ c㌧, 迦i ng殊 sau t邪 l竺 chｑh s雌h quan tr┥g....6989",pk::decode(pk::get_name(gunshi)))),gunshi);
							pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x trち xu字 \x1b[1x{}\x1b[0x ng㌻ l竺 t姐q蔀, thay t㍊ |Ⅰc n殊 qu■ c㌧ chi ｄ..6990",pk::decode(pk::get_name(gunshi)),pk::decode(pk::get_name(kunshu_t)))));
							kunshu_t.father = gunshi.get_id();
							kunshu_t.update();
						}
						else//王莽篡汉
						{
							pk::message_box(pk::encode(pk::format("Qu■ c㌧ b志 ngⅠc, chПg th□ 枝g l┃ ta \x1b[1x{}\x1b[0x 嫉 t■ qu■, |Пg ｃ b字 |〓 煙....6991",pk::decode(pk::get_name(gunshi)))),gunshi);
							pk::kill(kunshu_t);//处死剧情
							pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x p㍊ \x1b[1x{}\x1b[0x 嫉 t染 d■, thay t㍊ |Ⅰc n殊 qu■ c㌧ chi ｄ..6992",pk::decode(pk::get_name(gunshi)),pk::decode(pk::get_name(kunshu_t)))));
						}
					}
					else
					{
						//有宗室，霍光
						list_ket[0].mibun = 身份_君主;
						list_ket[0].update();
						force_t.kunshu = list_ket[0].get_id();
						force_t.update();
						if(kunshu_t.get_id() == 武将_曹髦)//曹髦必死
						{
							pk::kill(kunshu_t);//处死剧情
						}
						pk::message_box(pk::encode("C㌧ c＄g b志 ngⅠc, thロh |竺 nh■ |竺 hすh y Do脂 ho〓 quang viヶ!.6993"),reporter);
						pk::message_box(pk::encode("Nい hすh y Do脂 ho〓 quang viヶ, k△c l┃ t■ qu■, |Пg ｃ b字 |〓 煙....6994"),gunshi);
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x p㍊ \x1b[1x{}\x1b[0x 嫉 t染 d■, k△c l┃ t＄g th字 \x1b[1x{}\x1b[0x 嫉 qu■..6995",pk::decode(pk::get_name(gunshi)),pk::decode(pk::get_name(kunshu_t)),pk::decode(pk::get_name(list_ket[0])))));
					}
					pk::reset_taishu(pk::get_building(list_ket[0].service));
				}	
				else//君主独立
				{
					kunshu_t.mibun = 身份_一般;
					kunshu_t.rank = 80;
					kunshu_t.update();
					pk::move(gunshi,pk::get_building(pk::get_city_list(force_t)[1].get_id()));
					gunshi.mibun = 身份_君主;
					gunshi.update();
					force_t.kunshu = gunshi.get_id();
					force_t.gunshi= -1;
					force_t.update();
					pk::hanran(kunshu_t, true);
					pk::force@ force_new= pk::get_force(kunshu_t.get_force_id());
					五贼外交(force_new,-50,-50,-50,-50,-50);
					失去城市封地(pk::building_to_city(pk::get_building(kunshu_t.service)),force_t,force_new);
					if(force_t.title < 爵位_公)
					{
						force_new	.title = 爵位_公;
					}
					else if(force_t.title > 爵位_州牧)
					{
						force_new	.title = 9;
					}
					else 
					{
						force_new	.title = force_t.title +1;
					}
					force_new.update();
					pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x muズ |oΤ h趣 quyレ ｂc, 操 λ nざg ｂc ぅu hが, t㍊ ｂc ph■ liヱ..6996",pk::decode(pk::get_name(kunshu_t)))));
				}
				return;
			}
		}
		//分化：15%
		void AI_分化()
		{
			int num=pk::rand(100);
			if(num>5 and num<90) return;//不分化
			auto force_list = pk::list_to_array(pk::get_force_list());
			int nlength = pk::rand(force_list.length);//Roll一个随机数字
			pk::force@ force_t = force_list[nlength];
			if(pk::get_city_count(force_t)==1) return;//独城势力不分化
			//获取太守列表
			pk::list<pk::person@> list_t = pk::get_person_list(force_t, pk::mibun_flags(身份_太守));
			list_t.sort(function(a, b)
            			{
				return a.stat[武将能力_魅力]  > b.stat[武将能力_魅力] ;// 按魅力排序
			});
			if(list_t.count == 0) return;//无太守PASS
			bool 汉帝在否 = true;
			if(pk::get_scenario().emperor==-1) 汉帝在否 = false;
			int 选中太守 = pk::rand(list_t.count);
			if(force_t.is_player())//玩家被抽中
			{
				if(汉帝在否)
				{
					pk::person@ kunshu_t = pk::get_person(force_t.kunshu);
					if (pk::choose({ pk::encode("郡"), pk::encode("Kh＄g") }, pk::encode(pk::format("H⑮ 算 清c s雌h phong \x1b[1x{}\x1b[0x, hay kh＄g kh⑮g 精?\n.6997",pk::decode(pk::get_name(list_t[选中太守])))), kunshu_t) == 1)
					{
						pk::hanran(list_t[选中太守], true);//被抽中的太守独立
						list_t[选中太守].rank == 80;
						list_t[选中太守].update();
						pk::force@ force_new= pk::get_force(list_t[选中太守].get_force_id());
						失去城市封地(pk::building_to_city(pk::get_building(list_t[选中太守].service)),force_t,force_new);
						五贼外交(force_new,-50,-50,-50,-50,-50);
						pk::add_relations(force_new.get_id(),force_t.get_id(),80);
						pk::set_ally(force_new.get_id(),force_t.get_id(),true);
						force_new.title = 7 + pk::rand(3);//爵位随机
						string tname;
						if(force_new.title == 9) tname = "太守";
						else tname = pk::decode(pk::get_name(pk::get_title(force_new.title)));
						pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x c拙u h⑮ 算 s雌h phong 嫉 \x1b[1x{}\x1b[0x..6998",pk::decode(pk::get_name(list_t[选中太守])),tname)));
					}
					else
					{
						for(int i=0;i<list_t.count;i++)
						{
							if(pk::rand(2) == 0) continue;
							pk::message_box(pk::encode(pk::format("\x1b[1x{}\x1b[0x lΗ tr↓ thi⑤ t委chi mごh, 七y ta khД binh t∟o phΤ!.6999",pk::decode(pk::get_name(kunshu_t)))),list_t[i]);
							pk::hanran(list_t[i], false);//被抽中的太守独立
							list_t[i].rank == 80;
							list_t[i].update();
							pk::force@ force_new= pk::get_force(list_t[i].get_force_id());
							五贼外交(force_new,-50,-50,-50,-50,-50);
							force_new.title = 7 + pk::rand(3);//爵位随机
							pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x khД binh t∟o phΤ \x1b[1x{}\x1b[0x..6746",pk::decode(pk::get_name(list_t[i])),pk::decode(pk::get_name(kunshu_t)))));
						}
					}
				}
				else
				{
					pk::hanran(list_t[选中太守], false);//被抽中的太守独立
					list_t[选中太守].rank == 80;
					list_t[选中太守].update();
					pk::force@ force_new= pk::get_force(list_t[选中太守].get_force_id());
					五贼外交(force_new,-50,-50,-50,-50,30);
					force_new.title = 10;//爵位修正
					force_new.kokugou = 5;//国号黄巾
					pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x c拙u khざ vすg qu■ khuy⑤ b▽ ph旨 loが..7000",pk::decode(pk::get_name(list_t[选中太守])))));
				}
				return;
			}
			else
			{
				pk::hanran(list_t[选中太守], false);//被抽中的太守独立
				list_t[选中太守].rank == 80;
				list_t[选中太守].update();
				pk::force@ force_new= pk::get_force(list_t[选中太守].get_force_id());
				if(汉帝在否)
				{
					force_new.title = 7 + pk::rand(3);//爵位随机
					五贼外交(force_new,-50,-50,-50,-50,-50);
					string tname;
					if(force_new.title == 9) tname = "太守";
					else tname = pk::decode(pk::get_name(pk::get_title(force_new.title)));
					pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x c拙u h⑮ 算 s雌h phong 嫉 \x1b[1x{}\x1b[0x..6998",pk::decode(pk::get_name(list_t[选中太守])),tname)));
				}
				else
				{
					force_new.title = 10;//爵位修正
					force_new.kokugou = 5;//国号黄巾
					五贼外交(force_new,-50,-50,-50,-50,30);
					pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x c拙u khざ vすg qu■ khuy⑤ b▽ ph旨 loが..7000",pk::decode(pk::get_name(list_t[选中太守])))));
				}
				return;
			}
		}
		//节日：100%
		void AI_节日()
		{
			auto force_list = pk::list_to_array(pk::get_force_list());
			int nlength = pk::rand(force_list.length);//Roll一个随机数字
			pk::force@ force_t = force_list[nlength];
			pk::person@ kunshu_t = pk::get_person(force_t.kunshu);
			pk::building@ kunshu_building = pk::get_building(kunshu_t.service);
			pk::person@ reporter;
			if(force_t.gunshi == -1) @reporter = pk::get_person(武将_文官);
			else @reporter = pk::get_person(force_t.gunshi);
			@镜头 = kunshu_building;
			pk::scene(pk::scene_t(移动镜头));
			pk::message_box(pk::encode("彫ng ｃ Nguy⑤ 斤n, c㌧ c＄g n⑤ chじ ng新 p△o hoa c子g d■ hヘng nh⑳..7001"),reporter);
			pk::message_box(pk::encode("R字 h腫 仕ta..7002"),kunshu_t);			
			pk::create_effect(95,kunshu_building.get_pos(),null);//烟花
			ch::add_tp(force_t, 500, force_t.get_pos());
			pk::message_box(pk::encode(pk::format("前 杓o |iз gia tざg r趣 \x1b[1x{}\x1b[0x..7003", "500")));
			return;
		}
		//君主之死：约20%
		void AI_君死()
		{
			int num=pk::rand(100);
			auto force_list = pk::list_to_array(pk::get_force_list());
			int nlength = pk::rand(force_list.length);//Roll一个随机数字
			pk::force@ force_t = force_list[nlength];//随机一个势力
			pk::person@ kunshu_t = pk::get_person(force_t.kunshu);
			pk::building@ kunshu_building = pk::get_building(kunshu_t.service);			
			if(kunshu_t.death - pk::get_year() > 5)
			{
				if(num>10 and num<90) kunshu_t.shoubyou =伤病_轻症;
				else kunshu_t.shoubyou =伤病_重症;
				kunshu_t.update();
				pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x vｊ t■ ni⑤ tiヶ rⅠu trung uズg rⅠu q蔀 暇 ti← tｊ thすh tⅩ..7004", pk::decode(pk::get_name(kunshu_t)))));
				return;
			}
			else
			{
				@病君 = pk::get_person(force_t.kunshu);
				if(force_t.gunshi == -1) @军师 = pk::get_person(武将_文官);
				else @军师 = pk::get_person(force_t.gunshi);
				if(num>10 and num<90)
				{
					kunshu_t.shoubyou =伤病_重症;
					kunshu_t.update();
					pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x vｊ t■ ni⑤ tiヶ rⅠu trung uズg rⅠu q蔀 暇 ti← tｊ thすh tⅩ..7004", pk::decode(pk::get_name(kunshu_t)))));
				}
				else if(num<10)//医者剧情
				{
					//医者不在，暴死
					if(华佗.mibun == 身份_死亡 or pk::get_building(华佗.service).get_force_id() != force_t.get_id())
					{
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x vｊ t■ ni⑤ tiヶ rⅠu trung uズg rⅠu q蔀 暇 ti← tｊ thすh tⅩ, 操 λ tｏ y kh＄g 仁 ｆt q改 b志 vong..7005", pk::decode(pk::get_name(kunshu_t)))));
						pk::kill(kunshu_t);//处死剧情
					}
					else
					{
						if(force_t.is_player()) pk::scene(pk::scene_t(scene_医者来访));
						else
						{
							if(pk::rand(100)>50)//AI
							{
								pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("Y g狙 \x1b[1x{}\x1b[0x 嫉 \x1b[1x{}\x1b[0x gi社 ch社..7006", pk::decode(pk::get_name(华佗)),pk::decode(pk::get_name(kunshu_t)))));
								华佗.mibun = 身份_死亡;
								华佗.update();
								pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x t＠ tｐh tポ s〓, bごh tｐh ng殊 |思 tiご 持 b志 vong..7007", pk::decode(pk::get_name(kunshu_t)))));
								pk::kill(kunshu_t);//处死剧情
							}
							else
							{
								pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x bごh nыg, may 仁 tｏ 算n y g狙 \x1b[1x{}\x1b[0x, ti← tｊ chuy⑲ ∟o..7008", pk::decode(pk::get_name(kunshu_t)),pk::decode(pk::get_name(华佗)))));
								kunshu_t.death = kunshu_t.death + 5;
								kunshu_t.update();
							}
						}
					}
				}
				else//道人剧情
				{	
					//道人不在，暴死
					if(于吉.mibun == 身份_死亡 or pk::get_building(于吉.service).get_force_id() != force_t.get_id())
					{
						pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x Ｑm th⑮ t㊧ mごh bu＄g xuズg ti← tｊ thすh tⅩ, 操 λ tｏ ti⑤ kh＄g 仁 ｆt q改 b志 vong..7009", pk::decode(pk::get_name(kunshu_t)))));
						pk::kill(kunshu_t);//处死剧情
					}
					else
					{
						if(force_t.is_player()) pk::scene(pk::scene_t(scene_道人来访));
						else
						{
							pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x bごh nыg, d子g \x1b[1x{}\x1b[0x 餐 hi← chi h孜g hoす, ti← tｊ chuy⑲ ∟o, cすg thヵ t＠ tｐh tポ s〓..7010", pk::decode(pk::get_name(kunshu_t)),pk::decode(pk::get_name(于吉)))));
							kunshu_t.death = kunshu_t.death - 5;
							kunshu_t.update();
						}
					}
				}
				
			}			
			return;
		}
		void scene_医者来访()
		{
			pk::diplomacy(病君,军师,华佗, pk::diplomacy_t(Event_医者));
			return;
		}
		void Event_医者()
		{
			pk::message_box(pk::encode(pk::format("\x1b[1x{}\x1b[0x |竺 nh■ chi tⅩ, yそ c紫 chａ 炊 |紫 荏....7011",pk::decode(pk::get_name(病君)))),华佗);
			pk::message_box(pk::encode("C↓ 逝? Ng→i tｊ |em n殊 lang b朱 尺o xuズg chａ!.7012"),病君);
			pk::message_box(pk::encode(pk::format("C㌧ c＄g, \x1b[1x{}\x1b[0x ｃ th□ y, r字 仁 d■ v┥g, kh＄g tＺ 性t chi....7013",pk::decode(pk::get_name(华佗)))),军师);
			if (pk::choose({ pk::encode("Gi社 ch社 Hoa ㌢.7014"), pk::encode("Nghe theo lｉ dы c狩 雀c ｅ.7015") }, pk::encode("Nい n⑤ n人 t㍊ n事 quy社 |o⑮?.7016"), 病君) == 0)
			{
				华佗.mibun = 身份_死亡;
				华佗.update();
				pk::message_box(pk::encode("Ch移phち n振 nヤ!.7017"),病君);
				pk::message_box(pk::encode("K疏 k疏... Kh＄g ng㌻ 迅 trｉ 算n t＠ |ぞ....7018"),病君);				
				pk::kill(病君);//处死剧情
			}
			else
			{
				pk::message_box(pk::encode("T止 thｉ t⊥ 睡t l□..7019"),病君);
				if(pk::rand(100)>10)//AI
				{
					pk::message_box(pk::encode("K疏 k疏... Kh＄g ng㌻ 迅 trｉ 算n t＠ |ぞ....7018"),病君);		
					pk::kill(病君);//处死剧情
				}
				else
				{
					pk::message_box(pk::encode("Ti⑤ sinh thⅩ ｃ th□ y cТg!.7020"),病君);
					pk::message_box(pk::encode(pk::format("\x1b[1x{}\x1b[0x |竺 nh■ c□ t┏h tじ |iヰ dプng, kh＄g tＺ l竺 lΗ l死g v字 旧 q蔀 暇....7021",pk::decode(pk::get_name(病君)))),华佗);
					病君.death = 病君.death + 5;
					病君.update();
				}
			}
		}
		void scene_道人来访()
		{	
			pk::diplomacy(病君,军师,于吉, pk::diplomacy_t(Event_道人));
			return;
		}
		void Event_道人()
		{
			pk::message_box(pk::encode(pk::format("\x1b[1x{}\x1b[0x |竺 nh■ chi tⅩ, nい 仁 tam s雌h....7022",pk::decode(pk::get_name(病君)))),于吉);
			pk::message_box(pk::encode("ThⅠng s雌h: 測i nh■ c子g ta c子g |i tu hすh, 仁 tＺ trズ kiｚ nが n殊, duy⑤ t㊧ 睡t 蒲..7023"),于吉);
			pk::message_box(pk::encode("Trung s雌h: 景 v事 h孜g hoす, khズ k鹿 toす tiそ, nhブg n殊 bごh ng殊 sau l竺 p△t, hъ ph＊ ch社 kh＄g tＺ nghi n自..7024"),于吉);
			pk::message_box(pk::encode("ぉ s雌h: Nネ....7025"),于吉);
			int 选择 = pk::choose({pk::encode("Tho↓ ｄ tu hすh.7026"), pk::encode("景 v事 h孜g hoす.7027"),pk::encode("Gi社 ch社 yそ |志.7028")}, pk::encode("Ta n⑤ n人 t㍊ n事 ｂa ch┥.6816"), 病君);
			pk::force @force_t =pk::get_force(病君.get_force_id());
			if(选择 == 0)
			{
				//获取宗室列表
				pk::list<pk::person@> list_t = pk::get_person_list(force_t, pk::mibun_flags(身份_都督, 身份_太守, 身份_一般));
				list_t.sort(function(a, b)
            				{
					return a.birth < b.birth;// 按年龄大排序
				});
				pk::list<pk::person@> list_ket;//宗室
				for(int i=0;i<list_t.count;i++)
				{
					if(list_t[i].ketsuen == 病君.ketsuen) 
						list_ket.add(list_t[i]);
				}
				if(list_ket.count == 0)
				{
					pk::message_box(pk::encode("Nい nguyご Ｕ ti⑤ nh■ du, 操 λ t＄g th字 迦 tuyヱ, cた ph＊ ∟o ｂa ch┥ s雌h..7029"),病君);
					pk::message_box(pk::encode("Ti⑤ sinh thⅩ ｃ th□ nh■ vテ!.7030"),病君);
					病君.death = 病君.death - 5;
					病君.update();
				}
				else
				{
					pk::message_box(pk::encode("Nい nguyご Ｕ ti⑤ nh■ du..7031"),病君);
					pk::message_box(pk::encode(pk::format("Truyレ ta chi lごh, ng殊 sau Ｕ \x1b[1x{}\x1b[0x thay ta chi ｄ....7032",pk::decode(pk::get_name(list_ket[0])))),病君);
					pk::message_box(pk::encode("Tu■ mごh..6786"),军师);
					病君.mibun = 身份_一般;
					病君.absence_timer = 108;
					病君.death = 病君.death + 12;
					病君.update();
					list_ket[0].mibun = 身份_君主;
					list_ket[0].update();
					force_t.kunshu = list_ket[0].get_id();
					force_t.update();
				}
			}
			else if(选择 == 1)
			{
				pk::message_box(pk::encode("Ti⑤ sinh thⅩ ｃ th□ nh■ vテ!.7030"),病君);
				病君.death = 病君.death - 5;
				病君.update();
			}
			else
			{
				pk::message_box(pk::encode("Ng→i tｊ |em n殊 yそ nh■ 尺o xuズg chａ!.7033"),病君);
				于吉.mibun = 身份_死亡;
				于吉.update();
				pk::message_box(pk::encode("K疏 k疏... Kh＄g ng㌻ 迅 trｉ 算n t＠ |ぞ....7018"),病君);				
				pk::kill(病君);//处死剧情
			}
		}
		//四夷来朝:100%
		void AI_来朝()
		{
			if(pk::get_scenario().emperor==-1) return;
			@emperor = pk::get_person(pk::get_scenario().emperor);
			@镜头 = pk::get_building(emperor.service);
			pk::scene(pk::scene_t(移动镜头));
			拥帝势力编号 = pk::get_building(emperor.service).get_force_id();
			if(拥帝势力编号 == -1)
			{
				@权臣 = pk::get_person(武将_恶臣);
				@军师 = pk::get_person(武将_宦官);
			}
			else
			{
				@权臣 = pk::get_person(pk::get_force(拥帝势力编号).kunshu);
				if(pk::get_force(拥帝势力编号).gunshi == -1) @军师 = pk::get_person(武将_文官);
				else @军师 = pk::get_person(pk::get_force(拥帝势力编号).gunshi);
			}
			pk::scene(pk::scene_t(scene_四夷来朝));
			pk::history_log(pk::get_cursor_screen_pos(),-1, pk::encode(pk::format("\x1b[1x{}\x1b[0x triヰ ki← h⑮ 算..7034","四夷首领" )));
			return;
		}
		void scene_四夷来朝()
		{
			pk::council(emperor,权臣,军师,越君,乌君,羌君,蛮君,pk::council_t(council_四夷来朝));
			return;
		}
		void council_四夷来朝()
		{
			for (int i = 0; i < 四异首领.length; i++)
			{
				pk::person@ 异君 = pk::get_person(四异首领[i]);
				pk::force @异族 = pk::get_force(异君.get_force_id());
				if(异族.title == 1)//王要求赐汉室女
				{
					pk::message_box(pk::encode("Thロh h⑮ thi⑤ t委ban nλ H⑮ 勺!.7035"),异君);	
				}
				else if(异族.title == 2)//公要求封王赐姓刘
				{
					pk::message_box(pk::encode("Thロh h⑮ thi⑤ t委phong v〒ng ban ㊧!.7036"),异君);
				}
				else if(异族.title == 3)//大司马要求封公赐姓
				{
					pk::message_box(pk::encode("Thロh h⑮ thi⑤ t委phong c＄g ban danh!.7037"),异君);
				}
				else if(异族.title == 4)//大将军要求封大司马赐单字名
				{
					pk::message_box(pk::encode("Thロh h⑮ thi⑤ t委phong t祉c ban t拙!.7038"),异君);
				}
				else if(异族.title == 5)//诸侯要求封大司马赐双字名
				{
					pk::message_box(pk::encode("Thロh h⑮ thi⑤ t委t疏 t祉c ban danh!.7039"),异君);
				}
				else//大将军之前要求封爵
				{
					pk::message_box(pk::encode("Thロh h⑮ thi⑤ t委牲 quan!.7040"),异君);
				}
				pk::message_box(pk::encode("舘 khanh 仕刺 n人 t㍊ n事?.7041"),emperor);
				if(!pk::get_force(拥帝势力编号).is_player())
				{
					if(pk::rand(2)>0)
					{
						pk::message_box(pk::encode("Nhブg!"),权臣);
						pk::message_box(pk::encode("Nhブg..."),emperor);						
						if(异族.title == 1)
						{
							//暂时搁置
						}
						else if(异族.title == 2)
						{
							异君.sei = pk::encode("うu");
							异族.title = 1;
						}
						else if(异族.title == 3)
						{
							异君.mei =  pk::get_person(pk::rand(670)).mei;
							异族.title = 2;
						}
						else if(异族.title ==4)
						{
							异君.sei =  pk::get_person(pk::rand(670)).sei;
							异族.title = 3;
						}
						else if(异族.title ==5)
						{
							异君.mei =  pk::get_person(pk::rand(670)).mei+""+pk::get_person(pk::rand(670)).mei;
							异族.title = 4;
						}
						else 异族.title = 异族.title - 1;
						异君.update();
						异族.update();
						if(拥帝势力编号 != -1) pk::add_relations(拥帝势力编号,异族.get_id(),10);
						pk::message_box(pk::encode(pk::format("彫 t案\x1b[1x{}\x1b[0x |竺 nh■!.7044",pk::decode(pk::get_name(权臣)))),异君);
					}
					else
					{
						pk::message_box(pk::encode("薪 ng〒i ng→i n事! C杏逝 c＄g |モ?.7045"),权臣);
						pk::message_box(pk::encode("Kh＄g..."),emperor);
						if(拥帝势力编号 != -1) pk::add_relations(拥帝势力编号,异族.get_id(),-10);
						pk::message_box(pk::encode(pk::format("(\x1b[1x{}\x1b[0x, ng〒i c射! ).7047",pk::decode(pk::get_name(权臣)))),异君);
					}
				}
				else
				{
					if (pk::choose({ pk::encode("Chuハ"), pk::encode("Kh＄g") }, pk::encode("C㌧ c＄g n人 t㍊ n事 quy社 |o⑮?.7049"), 军师) == 0)
					{
						pk::message_box(pk::encode("Nhブg!"),权臣);
						pk::message_box(pk::encode("Nhブg..."),emperor);
						if(异族.title == 1)
						{
							//暂时搁置
						}
						else if(异族.title == 2)
						{
							异君.sei = pk::encode("うu");
							异族.title = 1;
						}
						else if(异族.title == 3)
						{
							异君.mei =  pk::get_person(pk::rand(670)).mei;
							异族.title = 2;
						}
						else if(异族.title ==4)
						{
							异君.sei =  pk::get_person(pk::rand(670)).sei;
							异族.title = 3;
						}
						else if(异族.title ==5)
						{
							异君.mei =  pk::get_person(pk::rand(670)).mei+""+pk::get_person(pk::rand(670)).mei;
							异族.title = 4;
						}
						else 异族.title = 异族.title - 1;
						异君.update();
						异族.update();
						pk::add_relations(拥帝势力编号,异族.get_id(),10);
						pk::message_box(pk::encode(pk::format("彫 t案\x1b[1x{}\x1b[0x |竺 nh■!.7044",pk::decode(pk::get_name(权臣)))),异君);
					}
					else
					{
						pk::message_box(pk::encode("薪 ng〒i ng→i n事! C杏逝 c＄g |モ?.7045"),权臣);
						pk::message_box(pk::encode("Kh＄g..."),emperor);
						 pk::add_relations(拥帝势力编号,异族.get_id(),-20);
						pk::message_box(pk::encode(pk::format("(\x1b[1x{}\x1b[0x, ng〒i c射! ).7047",pk::decode(pk::get_name(权臣)))),异君);
					}
				}	
			}
			return;
		}
		//四夷更替:10%/50%
		void AI_蛮易()
		{
			for (int i = 0; i < 四异首领.length; i++)
			{
				pk::person@ 异君 = pk::get_person(四异首领[i]);
				string 前异君 = pk::decode(pk::get_name(异君));
				pk::force @异族 = pk::get_force(异君.get_force_id());
				string 异族名 = pk::decode(pk::get_name(pk::get_kokugou(异族.kokugou)));				
				if(pk::get_year()%10 == 0)
				{
					if(pk::rand(2)>0) continue;
					异君.birth = pk::get_year() -15;
					异君.death = 异君.birth + 90;
					//父亲传子，先推恩后起名
					if(异族.title < 7)//推恩令
					{
						异族.title = 异族.title+1;
						异族.update();
					}
					if(异族.title >= 3) 异君.mei =  pk::get_person(pk::rand(670)).mei+""+pk::get_person(pk::rand(670)).mei;
					else 异君.mei = pk::get_person(pk::rand(670)).mei;
					pk::history_log(异族.get_pos(), 异族.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x t㌧ l┏h \x1b[1x{}\x1b[0x ch社 b字 |〓 k医益, n殊 t委\x1b[1x{}\x1b[0x ｆ thユ n殊 ｄ..7050",异族名,前异君,pk::decode(pk::get_name(异君)))));
				}
				else
				{
					//兄弟相传，先起名后推恩
					if(pk::rand(5)>0) continue;
					异君.birth = 异君.birth + 1;
					异君.death = 异君.birth + 90;
					if(异族.title >= 3) 异君.mei =  pk::get_person(pk::rand(670)).mei+""+pk::get_person(pk::rand(670)).mei;
					else 异君.mei = pk::get_person(pk::rand(670)).mei;
					if(异族.title < 7)//推恩令
					{
						异族.title = 异族.title+1;
						异族.update();
					}
					pk::history_log(异族.get_pos(), 异族.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x gi社 ch社 huynh trヘng \x1b[1x{}\x1b[0x, tr失thすh \x1b[1x{}\x1b[0x t㌧ l┏h..7051",pk::decode(pk::get_name(异君)),前异君,异族名)));
				}
			}
			return;
		}
		//狂士：10%
		void AI_狂士()
		{
			int num=pk::rand(100);
			if(num>5 and num<95) return;//不发生
			auto force_list = pk::list_to_array(pk::get_force_list());
			int nlength = pk::rand(force_list.length);//Roll一个随机数字
			pk::force@ force_t = force_list[nlength];
			if(pk::get_city_count(force_t)==1) return;//独城势力不发生
			//获取太守列表
			pk::list<pk::person@> list_t = pk::get_person_list(force_t, pk::mibun_flags(身份_太守));
			if(list_t.count == 0) return;//无太守PASS
			if(祢衡.mibun == 身份_死亡) return;//狂士已死
			int 选中太守 = pk::rand(list_t.count);
			if(list_t[选中太守].character <=1)
			{
				if(list_t[选中太守].base_stat[武将能力_武力] > 5)
				{
					list_t[选中太守].base_stat[武将能力_武力] = list_t[选中太守].base_stat[武将能力_武力] - 5;
				}
				else list_t[选中太守].base_stat[武将能力_武力]  = 0;
				list_t[选中太守].base_stat[武将能力_魅力] = list_t[选中太守].base_stat[武将能力_魅力] + 1;
				list_t[选中太守].update();
				pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x 嫉 \x1b[1x{}\x1b[0x 餐 nhち 星 kh＄g dao 暇ng ( v散ｂc -5, m畏ｂc +1)..7052",pk::decode(pk::get_name(list_t[选中太守])),pk::decode(pk::get_name(祢衡)))));
			}
			else
			{
				list_t[选中太守].base_stat[武将能力_武力] = list_t[选中太守].base_stat[武将能力_武力] + 1;
				if(list_t[选中太守].base_stat[武将能力_魅力] > 5)
				{
					list_t[选中太守].base_stat[武将能力_魅力] = list_t[选中太守].base_stat[武将能力_魅力] - 5;
				}
				else list_t[选中太守].base_stat[武将能力_魅力]  = 0;
				list_t[选中太守].update();
				祢衡.mibun = 身份_死亡;
				祢衡.update();
				pk::history_log(force_t.get_pos(), force_t.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x 嫉 \x1b[1x{}\x1b[0x 餐 nhち 星 gi＠ 性t chi ( v散ｂc +1, m畏ｂc -5)..7053",pk::decode(pk::get_name(list_t[选中太守])),pk::decode(pk::get_name(祢衡)))));
			}
			return;
		}
		//获取空城表
		pk::list<pk::building @> selectable_building_list_for_null()
		{
			pk::list<pk::building @> selectable_building_list;
			for (int i = 0; i < 城市_末; i++)
			{
				pk::building @ building_t = pk::get_building (i);
				if (building_t.get_force_id() == -1)
				{
					selectable_building_list.add(building_t);
				}
			}
			return selectable_building_list;
		}
		//独立：20%
		void AI_独立()
		{
			int num=pk::rand(100);
			if(num>10 and num<90) return;//不独立
			pk::list<pk::building @>空城列表 = selectable_building_list_for_null();
			if(空城列表.count == 0 ) return;//无空城
			int n = pk::rand(空城列表.count);//Roll一个随机数字
			pk::building @ building_t = 空城列表[n];
			pk::list<pk::person@> 在野列表 = pk::get_person_list(building_t, pk::mibun_flags(身份_在野));
			if(在野列表.count == 0 ) return;//无在野
			在野列表.sort(function(a, b)
            {
					return a.ambition < b.ambition;// 按野心排序
			});
			pk::hataage(在野列表[0],-1,-1,9,0);
			pk::add_gold(building_t,3600);
			pk::add_food(building_t,36000);
			pk::add_troops(building_t,18000);
			pk::add_energy(building_t,70);
			pk::add_weapon_amount(building_t,1,5000);
			pk::add_weapon_amount(building_t,2,5000);
			pk::force@ force_new= pk::get_force(在野列表[0].get_force_id());
			五贼外交(force_new,-50,-50,-50,-50,-50);
			pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x ㈹ |∵ ph〒ng c→ng λo 枝g 嫉 {} th↓ t㎏..7054",pk::decode(pk::get_name(在野列表[0])),pk::decode(pk::get_name(building_t)))));
			return;
		}
		//左迁：15%
		void AI_左迁()
		{
			int num=pk::rand(100);
			if(num>5 and num<90) return;//不左迁
			@emperor = pk::get_person(pk::get_scenario().emperor);
			if(pk::get_scenario().emperor == -1) return;
			pk::list<pk::building @>空城列表 = selectable_building_list_for_null();
			if(空城列表.count == 0 ) return;//无空城
			int n = pk::rand(空城列表.count);//Roll一个随机数字
			pk::building @ building_t = 空城列表[n];
			auto force_list = pk::list_to_array(pk::get_force_list());
			int nlength = pk::rand(force_list.length);//Roll一个随机数字
			//pk::force@ force_t = pk::get_force(1);//测试接口
			pk::force@ force_t = force_list[nlength];
			pk::list<pk::person@> list_1 = pk::get_person_list(force_t, pk::mibun_flags(身份_一般));//一般
			if(list_1.count == 0 ) return;//无一般
			list_1.sort(function(a, b)
            {
				return a.kouseki > b.kouseki;// 按功绩排序
			});
			pk::list<pk::person@> list_t;//无职
			for(int i=0;i<list_1.count;i++)
			{
				if(list_1[i].rank == 80 and list_1[i].get_id() != force_t.gunshi) 
					list_t.add(list_1[i]);
			}
			if(list_t.count == 0 ) return;//无闲置
			if(list_t[0].kouseki < 10000) return;//功绩不够
			//无害化处理
			list_t[0].district= -1;
			list_t[0].location = building_t.get_id();
			list_t[0].mibun = 身分_在野;
			list_t[0].loyalty = 0;
			list_t[0].order = 武将任务_无;					
			list_t[0].action_done = true;
			list_t[0].update();
			pk::move(list_t[0], building_t);//移动
			pk::hataage(list_t[0],-1,-1,9,0);
			pk::add_gold(building_t,3600);
			pk::add_food(building_t,36000);
			pk::add_troops(building_t,18000);
			pk::add_energy(building_t,70);
			pk::add_weapon_amount(building_t,1,5000);
			pk::add_weapon_amount(building_t,2,5000);			
			pk::force@ force_new= pk::get_force(list_t[0].get_force_id());
			五贼外交(force_new,-50,-50,-50,-50,-50);
			pk::add_relations(force_new.get_id(),force_t.get_id(),80);
			pk::set_ally(force_new.get_id(),force_t.get_id(),true);
			pk::city @空城 =pk::building_to_city(building_t);
			获取空城封地(空城,force_new);			
			pk::history_log(force_new.get_pos(), force_new.color, pk::encode(pk::format("\x1b[1x{}\x1b[0x c拙u h⑮ 算 nhじ mごh, dｉ 嫉 {} th↓ t㎏..7055",pk::decode(pk::get_name(list_t[0])),pk::decode(pk::get_name(空城)))));
			return;
		}
		//勒索：10%
		void AI_勒索()
		{
			int num=pk::rand(100);
			if(num>5 and num<95) return;
			return;
		}
	}
	Main main;
}