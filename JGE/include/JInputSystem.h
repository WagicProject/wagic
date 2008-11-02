#ifndef _JINPUTSYSTEM_H_
#define _JINPUTSYSTEM_H_

#include "JLBFont.h"

extern char input_table[3][9][4];

struct PY_index
	{
		char *PY;
		char *PY_mb;
	};
//"Æ´ÒôÊäÈE¨ºº×ÖÅÅÁĞ±EÂEEmb)"
static char PY_mb_a[]     ={""};
static char PY_mb_ai[]    ={""};
static char PY_mb_an[]    ={""};
static char PY_mb_ang[]   ={""};
static char PY_mb_ao[]    ={""};
static char PY_mb_ba[]    ={""};
static char PY_mb_bai[]   ={""};
static char PY_mb_ban[]   ={""};
static char PY_mb_bang[]  ={""};
static char PY_mb_bao[]   ={""};
static char PY_mb_bei[]   ={""};
static char PY_mb_ben[]   ={"±¼±¾±½±¿º»"};
static char PY_mb_beng[]  ={"±À±Á±Â±Ã±Å±Ä"};
static char PY_mb_bi[]    ={"±Æ±Ç±È±Ë±Ê±É±Ò±Ø±Ï±Õ±Ó±Ñ±İ±Ğ±Ö±Ô±Í±×±Ì±Î±Ú±Ü±Û"};
static char PY_mb_bian[]  ={""};
static char PY_mb_biao[]  ={""};
static char PY_mb_bie[]   ={""};
static char PY_mb_bin[]   ={""};
static char PY_mb_bing[]  ={"±ù±ø±û±Eú±ş±ı²¢²¡"};
static char PY_mb_bo[]    ={"²¦²¨²£²§²±²¤²¥²®²µ²¯²´²ª²¬²°²©²³²«²­²²²·"};
static char PY_mb_bu[]    ={"²¹²¸²¶²»²¼²½²À²¿²º²¾"};
static char PY_mb_ca[]    ={"²Á"};
static char PY_mb_cai[]   ={"²Â²Å²Ä²Æ²Ã²É²Ê²Ç²È²Ë²Ì"};
static char PY_mb_can[]   ={"²Î²Í²Ğ²Ï²Ñ²Ò²Ó"};
static char PY_mb_cang[]  ={"²Ö²×²Ô²Õ²Ø"};
static char PY_mb_cao[]   ={"²Ù²Ú²Ü²Û²İ"};
static char PY_mb_ce[]    ={"²á²à²Ş²â²ß"};
static char PY_mb_ceng[]  ={""};
static char PY_mb_cha[]   ={"²æ²å²é²ç²è²EEEúÎEûå²"};
static char PY_mb_chai[]  ={""};
static char PY_mb_chan[]  ={""};
static char PY_mb_chang[] ={"²ı²ş³¦³¢³¥³£³§³¡³¨³©³«³ª"};
static char PY_mb_chao[]  ={"³­³®³¬³²³¯³°³±³³³´´Â"};
static char PY_mb_che[]   ={"³µ³¶³¹³¸³·³º"};
static char PY_mb_chen[]  ={"³»³¾³¼³À³Á³½³Â³¿³Ä³Ã"};
static char PY_mb_cheng[] ={"³Æ³Å³É³Ê³Ğ³Ï³Ç³Ë³Í³Ì³Î³È³Ñ³Ò³Ó"};
static char PY_mb_chi[]   ={""};
static char PY_mb_chong[] ={""};
static char PY_mb_chou[]  ={""};
static char PY_mb_chu[]   ={""};
static char PY_mb_chuai[] ={"´§"};
static char PY_mb_chuan[] ={"´¨´©´«´¬´ª´­´®"};
static char PY_mb_chuang[]={"´³´¯´°´²´´"};
static char PY_mb_chui[]  ={"´µ´¶´¹´·´¸"};
static char PY_mb_chun[]  ={"´º´»´¿´½´¾´¼´À"};
static char PY_mb_chuo[]  ={"´Á"};
static char PY_mb_ci[]    ={"´Ã´Ê´Ä´É´È´Ç´Å´Æ´Ë´Î´Ì´Í"};
static char PY_mb_cong[]  ={"´Ñ´Ó´Ò´Ğ´Ï´Ô"};
static char PY_mb_cou[]   ={"´Õ"};
static char PY_mb_cu[]    ={"´Ö´Ù´×´Ø"};
static char PY_mb_cuan[]  ={"´Ú´Ü´Û"};
static char PY_mb_cui[]   ={""};
static char PY_mb_cun[]   ={""};
static char PY_mb_cuo[]   ={""};
static char PY_mb_da[]    ={""};
static char PY_mb_dai[]   ={""};
static char PY_mb_dan[]   ={"µ¤µ¥µ£µ¢µ¦µ¨µ§µ©µ«µ®µ¯µ¬µ­µ°µª"};
static char PY_mb_dang[]  ={"µ±µ²µ³µ´µµ"};
static char PY_mb_dao[]   ={"µ¶µ¼µºµ¹µ·µ»µ¸µ½µ¿µÁµÀµ¾"};
static char PY_mb_de[]    ={"µÃµÂµÄ"};
static char PY_mb_deng[]  ={"µÆµÇµÅµÈµËµÊµÉ"};
static char PY_mb_di[]    ={"µÍµÌµÎµÒµÏµĞµÓµÑµÕµ×µÖµØµÜµÛµİµÚµŞµÙ"};
static char PY_mb_dian[]  ={""};
static char PY_mb_diao[]  ={""};
static char PY_mb_die[]   ={""};
static char PY_mb_ding[]  ={"¶¡¶£¶¢¶¤¶¥¶¦¶©¶¨¶§"};
static char PY_mb_diu[]   ={"¶ª"};
static char PY_mb_dong[]  ={"¶«¶¬¶­¶®¶¯¶³¶±¶²¶°¶´"};
static char PY_mb_dou[]   ={"¶¼¶µ¶·¶¶¶¸¶¹¶º¶»"};
static char PY_mb_du[]    ={"¶½¶¾¶Á¶¿¶À¶Â¶Ä¶Ã¶Ê¶Å¶Ç¶È¶É¶Æ"};
static char PY_mb_duan[]  ={"¶Ë¶Ì¶Î¶Ï¶Ğ¶Í"};
static char PY_mb_dui[]   ={"¶Ñ¶Ó¶Ô¶Ò"};
static char PY_mb_dun[]   ={"¶Ö¶Ø¶Õ¶×¶Ü¶Û¶Ù¶İ"};
static char PY_mb_duo[]   ={""};
static char PY_mb_e[]     ={""};
static char PY_mb_en[]    ={""};
static char PY_mb_er[]    ={"¶ù¶ø¶û¶ú¶ı¶Eş·¡"};
static char PY_mb_fa[]    ={"·¢·¦·¥·£·§·¤·¨·©"};
static char PY_mb_fan[]   ={"·«·¬·­·ª·²·¯·°·³·®·±·´·µ·¸·º·¹·¶··"};
static char PY_mb_fang[]  ={"·½·»·¼·À·Á·¿·¾·Â·Ã·Ä·Å"};
static char PY_mb_fei[]   ={"·É·Ç·È·Æ·Ê·Ë·Ì·Í·Ï·Ğ·Î·Ñ"};
static char PY_mb_fen[]   ={""};
static char PY_mb_feng[]  ={""};
static char PY_mb_fo[]    ={""};
static char PY_mb_fou[]   ={""};
static char PY_mb_fu[]    ={"·ò·ô·õ·ó¸¥·Eö·÷·ş·ı·ú¸¡¸¢·û¸¤·ù¸£·ø¸§¸¦¸®¸«¸©¸ª¸¨¸­¸¯¸¸¸¼¸¶¸¾¸º¸½¸À¸·¸´¸°¸±¸µ¸»¸³¸¿¸¹¸²"};
static char PY_mb_ga[]    ={"¸Â¸Á"};
static char PY_mb_gai[]   ={"¸Ã¸Ä¸Æ¸Ç¸È¸Å"};
static char PY_mb_gan[]   ={"¸É¸Ê¸Ë¸Î¸Ì¸Í¸Ñ¸Ï¸Ò¸Ğ¸Ó"};
static char PY_mb_gang[]  ={"¸Ô¸Õ¸Ú¸Ù¸Ø¸×¸Ö¸Û¸Ü"};
static char PY_mb_gao[]   ={""};
static char PY_mb_ge[]    ={"¸EúÔç¸EEûÔé¸è¸ó¸Eñ¸ğ¸ô¸ö¸÷¸õ¿©"};
static char PY_mb_gei[]   ={""};
static char PY_mb_gen[]   ={""};
static char PY_mb_geng[]  ={"¸Eı¸û¸ş¹¡¹¢¹£"};
static char PY_mb_gong[]  ={"¹¤¹­¹«¹¦¹¥¹©¹¬¹§¹ª¹¨¹®¹¯¹°¹²¹±"};
static char PY_mb_gou[]   ={"¹´¹µ¹³¹·¹¶¹¹¹º¹¸¹»"};
static char PY_mb_gu[]    ={"¹À¹¾¹Ã¹Â¹Á¹½¹¼¹¿¹Å¹È¹É¹Ç¹Æ¹Ä¹Ì¹Ê¹Ë¹Í"};
static char PY_mb_gua[]   ={"¹Ï¹Î¹Ğ¹Ñ¹Ò¹Ó"};
static char PY_mb_guai[]  ={"¹Ô¹Õ¹Ö"};
static char PY_mb_guan[]  ={"¹Ø¹Û¹Ù¹Ú¹×¹İ¹Ü¹á¹ß¹à¹Ş"};
static char PY_mb_guang[] ={""};
static char PY_mb_gui[]   ={""};
static char PY_mb_gun[]   ={""};
static char PY_mb_guo[]   ={"¹ù¹ø¹ú¹û¹Eı"};
static char PY_mb_ha[]    ={"¸ò¹ş"};
static char PY_mb_hai[]   ={"º¢º¡º£º¥º§º¦º¤"};
static char PY_mb_han[]   ={"º¨º©º¬ºªº¯º­º®º«º±º°ººº¹ºµº·º´º¸º¶º³º²"};
static char PY_mb_hang[]  ={"º¼º½ĞĞ"};
static char PY_mb_hao[]   ={"ºÁºÀº¿º¾ºÃºÂºÅºÆºÄ"};
static char PY_mb_he[]    ={"ºÇºÈºÌºÏºÎºÍºÓºÒºËºÉºÔºĞºÊºØºÖºÕº×"};
static char PY_mb_hei[]   ={"ºÚºÙ"};
static char PY_mb_hen[]   ={"ºÛºÜºİºŞ"};
static char PY_mb_heng[]  ={""};
static char PY_mb_hong[]  ={""};
static char PY_mb_hou[]   ={""};
static char PY_mb_hu[]    ={"ºõºôºö»¡ºEúºøºşºùº÷ºıºû»¢»£»¥»§»¤»¦"};
static char PY_mb_hua[]   ={"»¨»ª»©»¬»«»¯»®»­»°"};
static char PY_mb_huai[]  ={"»³»²»´»±»µ"};
static char PY_mb_huan[]  ={"»¶»¹»·»¸»º»Ã»Â»½»»»Á»¼»À»¾»¿"};
static char PY_mb_huang[] ={"»Ä»Å»Ê»Ë»Æ»Ì»Í»È»Ç»É»Ğ»Î»Ñ»Ï"};
static char PY_mb_hui[]   ={"»Ò»Ö»Ó»Ô»Õ»Ø»×»Ú»Ü»ã»á»ä»æ»å»â»ß»Ş»à»İ»Ù»Û"};
static char PY_mb_hun[]   ={""};
static char PY_mb_huo[]   ={""};
static char PY_mb_ji[]    ={""};
static char PY_mb_jia[]   ={"¼Ó¼Ğ¼Ñ¼Ï¼Ò¼Î¼Ô¼Õ¼×¼Ö¼Ø¼Û¼İ¼Ü¼Ù¼Ş¼ÚĞ®"};
static char PY_mb_jian[]  ={"¼é¼â¼á¼ß¼ä¼ç¼è¼æ¼à¼ã¼Eå¼ğ¼ó¼úØEñ¼õ¼ô¼EEò¼ûØû¼ş½¨½¤½£¼ö¼ú½¡½§½¢½¥½¦¼ù¼ø¼Eı"};
static char PY_mb_jiang[] ={"½­½ª½«½¬½©½®½²½±½°½¯½³½µ½´"};
static char PY_mb_jiao[]  ={"½»½¼½¿½½½¾½º½·½¹½¶½¸½Ç½Æ½Ê½È½Ã½Å½Â½Á½Ë½É½Ğ½Î½Ï½Ì½Ñ½Í¾õ½À"};
static char PY_mb_jie[]   ={""};
static char PY_mb_jin[]   ={""};
static char PY_mb_jing[]  ={"¾©¾­¾¥¾£¾ª¾§¾¦¾¬¾¤¾«¾¨¾®¾±¾°¾¯¾»¾¶¾·¾º¾¹¾´¾¸¾³¾²¾µ"};
static char PY_mb_jiong[] ={"¾¼¾½"};
static char PY_mb_jiu[]   ={"¾À¾¿¾¾¾Å¾Ã¾Ä¾Á¾Â¾Æ¾É¾Ê¾Ì¾Î¾Ç¾È¾Í¾Ë"};
static char PY_mb_ju[]    ={""};
static char PY_mb_juan[]  ={""};
static char PY_mb_jue[]   ={""};
static char PY_mb_jun[]   ={"¾Eı¾ù¾û¾ú¿¡¿¤¾ş¿£¿¥¿¢"};
static char PY_mb_ka[]    ={"¿§¿¦¿¨"};
static char PY_mb_kai[]   ={"¿ª¿«¿­¿®¿¬"};
static char PY_mb_kan[]   ={"¼÷¿¯¿±¿°¿²¿³¿´"};
static char PY_mb_kang[]  ={"¿µ¿¶¿·¿¸¿º¿¹¿»"};
static char PY_mb_kao[]   ={"¿¼¿½¿¾¿¿"};
static char PY_mb_ke[]    ={"¿À¿Á¿Â¿Æ¿Ã¿Å¿Ä¿Ç¿È¿É¿Ê¿Ë¿Ì¿Í¿Î"};
static char PY_mb_ken[]   ={"¿Ï¿Ñ¿Ò¿Ğ"};
static char PY_mb_keng[]  ={"¿Ô¿Ó"};
static char PY_mb_kong[]  ={"¿Õ¿×¿Ö¿Ø"};
static char PY_mb_kou[]   ={"¿Ù¿Ú¿Û¿Ü"};
static char PY_mb_ku[]    ={""};
static char PY_mb_kua[]   ={""};
static char PY_mb_kuai[]  ={""};
static char PY_mb_kuan[]  ={""};
static char PY_mb_kuang[] ={""};
static char PY_mb_kui[]   ={"¿÷¿ù¿ø¿ú¿Eû¿ı¿şÀ¢À£À¡"};
static char PY_mb_kun[]   ={"À¤À¥À¦À§"};
static char PY_mb_kuo[]   ={"À©À¨À«Àª"};
static char PY_mb_la[]    ={"À¬À­À²À®À°À¯À±"};
static char PY_mb_lai[]   ={"À´À³Àµ"};
static char PY_mb_lan[]   ={"À¼À¹À¸À·À»À¶À¾À½ÀºÀÀÀ¿ÀÂÀÁÀÃÀÄ"};
static char PY_mb_lang[]  ={"ÀÉÀÇÀÈÀÅÀÆÀÊÀËòë"};
static char PY_mb_lao[]   ={"ÀÌÀÍÀÎÀÏÀĞÀÑÀÔÀÓÀÒ"};
static char PY_mb_le[]    ={"ÀÖÀÕÁË"};
static char PY_mb_lei[]   ={"À×ÀØÀİÀÚÀÙÀÜÀßÀáÀàÀÛÀŞ"};
static char PY_mb_leng[]  ={""};
static char PY_mb_li[]    ={"ÀåÀæÀEEòÀçÀE§ÀèÀéÀñÀûÜE¨ÀúÜğÁ¦ÀúÀ÷Á¢ÀôÀöÀûÀøÁ¤ÀıÁ¥ÀşÀóÀõÀùÁ£ÀE¡"};
static char PY_mb_lian[]  ={"Á¬Á±Á¯Á°Á«ÁªÁ®Á­Á²Á³Á·Á¶ÁµÁ´"};
static char PY_mb_liang[] ={"Á©Á¼Á¹ÁºÁ¸Á»Á½ÁÁÁÂÁ¾ÁÀÁ¿"};
static char PY_mb_liao[]  ={"ÁÊÁÉÁÆÁÄÁÅÁÈÁÎÁÃÁÇÁÍÁÏÁÌ"};
static char PY_mb_lie[]   ={"ÁĞÁÓÁÒÁÔÁÑ"};
static char PY_mb_lin[]   ={""};
static char PY_mb_ling[]  ={""};
static char PY_mb_liu[]   ={""};
static char PY_mb_long[]  ={"ÁúÁEıÁûÂ¡ÁşÂ¤Â¢Â£"};
static char PY_mb_lou[]   ={"Â¦Â¥Â§Â¨ÂªÂ©"};
static char PY_mb_lu[]    ={"Â¶Â¬Â®Â«Â¯Â­Â±Â²Â°Â³Â½Â¼Â¸Â¹Â»ÂµÂ·Â¾ÂºÂ´"};
static char PY_mb_luan[]  ={"ÂÏÂÍÂÎÂĞÂÑÂÒ"};
static char PY_mb_lue[]   ={"ÂÓÂÔ"};
static char PY_mb_lun[]   ={"ÂÕÂØÂ×ÂÙÂÚÂÖÂÛ"};
static char PY_mb_luo[]   ={""};
static char PY_mb_lv[]    ={"ÂËÂ¿ÂÀÂÂÂÃÂÁÂÅÂÆÂÄÂÉÂÇÂÊÂÌÂÈ"};
static char PY_mb_ma[]    ={""};
static char PY_mb_mai[]   ={""};
static char PY_mb_man[]   ={""};
static char PY_mb_mang[]  ={"Ã¦Ã¢Ã¤Ã£Ã§Ã¥"};
static char PY_mb_mao[]   ={"Ã¨Ã«Ã¬Ã©ÃªÃ®Ã­Ã¯Ã°Ã³Ã±Ã²"};
static char PY_mb_me[]    ={"Ã´"};
static char PY_mb_mei[]   ={"Ã»Ã¶ÃµÃ¼Ã·Ã½ÃºÃ¸Ã¹Ã¿ÃÀÃ¾ÃÃÃÁÃÄÃÂ"};
static char PY_mb_men[]   ={"ÃÅÃÆÃÇ"};
static char PY_mb_meng[]  ={"ÃÈÃËÃÊÃÍÃÉÃÌÃÏÃÎ"};
static char PY_mb_mi[]    ={"ÃÖÃÔÃÕÃÑÃÓÃÒÃ×ÃĞÃÚÃÙÃØÃÜÃİÃÛ"};
static char PY_mb_mian[]  ={""};
static char PY_mb_miao[]  ={""};
static char PY_mb_mie[]   ={""};
static char PY_mb_min[]   ={""};
static char PY_mb_ming[]  ={""};
static char PY_mb_miu[]   ={"Ãı"};
static char PY_mb_mo[]    ={"ºÑÃşÄ¡Ä£Ä¤Ä¦Ä¥Ä¢Ä§Ä¨Ä©Ä­Ä°ÄªÄ¯Ä®Ä«Ä¬"};
static char PY_mb_mou[]   ={"Ä²Ä±Ä³"};
static char PY_mb_mu[]    ={"Ä¸Ä¶ÄµÄ·Ä´Ä¾Ä¿ÄÁÄ¼Ä¹Ä»ÄÀÄ½ÄºÄÂ"};
static char PY_mb_na[]    ={"ÄÃÄÄÄÇÄÉÄÈÄÆÄÅ"};
static char PY_mb_nai[]   ={"ÄËÄÌÄÊÄÎÄÍ"};
static char PY_mb_nan[]   ={"ÄĞÄÏÄÑ"};
static char PY_mb_nang[]  ={"ÄÒ"};
static char PY_mb_nao[]   ={"ÄÓÄÕÄÔÄÖÄ×"};
static char PY_mb_ne[]    ={"ÄØ"};
static char PY_mb_nei[]   ={"ÄÚÄÙ"};
static char PY_mb_nen[]   ={"ÄÛ"};
static char PY_mb_neng[]  ={"ÄÜ"};
static char PY_mb_ni[]    ={""};
static char PY_mb_nian[]  ={""};
static char PY_mb_niang[] ={""};
static char PY_mb_niao[]  ={""};
static char PY_mb_nie[]   ={""};
static char PY_mb_nin[]   ={""};
static char PY_mb_ning[]  ={"ÄşÅ¡ÄEûÄıÅ¢"};
static char PY_mb_niu[]   ={"Å£Å¤Å¦Å¥"};
static char PY_mb_nong[]  ={"Å©Å¨Å§Åª"};
static char PY_mb_nu[]    ={"Å«Å¬Å­"};
static char PY_mb_nuan[]  ={"Å¯"};
static char PY_mb_nue[]   ={"Å±Å°"};
static char PY_mb_nuo[]   ={"Å²ÅµÅ³Å´"};
static char PY_mb_nv[]    ={"Å®"};
static char PY_mb_o[]     ={"Å¶"};
static char PY_mb_ou[]    ={"Å·Å¹Å¸Å»Å¼ÅºÅ½"};
static char PY_mb_pa[]    ={"Å¿Å¾ÅÀ°ÒÅÃÅÁÅÂ"};
static char PY_mb_pai[]   ={"ÅÄÅÇÅÅÅÆÅÉÅÈ"};
static char PY_mb_pan[]   ={"ÅËÅÊÅÌÅÍÅĞÅÑÅÎÅÏ"};
static char PY_mb_pang[]  ={"ÅÒÅÓÅÔÅÕÅÖ"};
static char PY_mb_pao[]   ={"Å×ÅÙÅØÅÚÅÛÅÜÅİ"};
static char PY_mb_pei[]   ={""};
static char PY_mb_pen[]   ={""};
static char PY_mb_peng[]  ={""};
static char PY_mb_pi[]    ={"±ÙÅúÅ÷ÅûÅøÅEùÆ¤ÅşÆ£Æ¡ÅıÆ¢Æ¥Æ¦Æ¨Æ§Æ©"};
static char PY_mb_pian[]  ={"Æ¬Æ«ÆªÆ­"};
static char PY_mb_piao[]  ={"Æ¯Æ®Æ°Æ±"};
static char PY_mb_pie[]   ={"Æ²Æ³"};
static char PY_mb_pin[]   ={"Æ´Æ¶ÆµÆ·Æ¸"};
static char PY_mb_ping[]  ={"Æ¹Æ½ÆÀÆ¾ÆºÆ»ÆÁÆ¿Æ¼"};
static char PY_mb_po[]    ={"ÆÂÆÃÆÄÆÅÆÈÆÆÆÉÆÇ"};
static char PY_mb_pou[]   ={"ÆÊ"};
static char PY_mb_pu[]    ={"¸¬ÆÍÆËÆÌÆÎÆĞÆÏÆÑÆÓÆÔÆÒÆÖÆÕÆ×ÆØ"};
static char PY_mb_qi[]    ={""};
static char PY_mb_qia[]   ={"ÆşÇ¡Ç¢"};
static char PY_mb_qian[]  ={"Ç§ÇªÇ¤Ç¨Ç¥Ç£Ç¦Ç«Ç©Ç°Ç®Ç¯Ç¬Ç±Ç­Ç³Ç²Ç´Ç·ÇµÇ¶Ç¸"};
static char PY_mb_qiang[] ={"ÇºÇ¼Ç¹Ç»Ç¿Ç½Ç¾ÇÀ"};
static char PY_mb_qiao[]  ={"ÇÄÇÃÇÂÇÁÇÇÇÈÇÅÇÆÇÉÇÎÇÍÇÏÇÌÇËÇÊ"};
static char PY_mb_qie[]   ={"ÇĞÇÑÇÒÇÓÇÔ"};
static char PY_mb_qin[]   ={"Ç×ÇÖÇÕÇÛÇØÇÙÇİÇÚÇÜÇŞÇß"};
static char PY_mb_qing[]  ={""};
static char PY_mb_qiong[] ={""};
static char PY_mb_qiu[]   ={""};
static char PY_mb_qu[]    ={"ÇøÇúÇıÇEùÇûÇ÷ÇşÈ¡È¢È£È¥È¤"};
static char PY_mb_quan[]  ={"È¦È«È¨ÈªÈ­È¬È©È§È®È°È¯"};
static char PY_mb_que[]   ={"È²È±È³È´È¸È·ÈµÈ¶"};
static char PY_mb_qun[]   ={"È¹Èº"};
static char PY_mb_ran[]   ={"È»È¼È½È¾"};
static char PY_mb_rang[]  ={"È¿ÈÂÈÀÈÁÈÃ"};
static char PY_mb_rao[]   ={"ÈÄÈÅÈÆ"};
static char PY_mb_re[]    ={"ÈÇÈÈ"};
static char PY_mb_ren[]   ={"ÈËÈÊÈÉÈÌÈĞÈÏÈÎÈÒÈÑÈÍ"};
static char PY_mb_reng[]  ={"ÈÓÈÔ"};
static char PY_mb_ri[]    ={"ÈÕ"};
static char PY_mb_rong[]  ={"ÈÖÈŞÈ×ÈÙÈİÈÜÈØÈÛÈÚÈß"};
static char PY_mb_rou[]   ={""};
static char PY_mb_ru[]    ={""};
static char PY_mb_ruan[]  ={""};
static char PY_mb_rui[]   ={""};
static char PY_mb_run[]   ={""};
static char PY_mb_ruo[]   ={""};
static char PY_mb_sa[]    ={""};
static char PY_mb_sai[]   ={""};
static char PY_mb_san[]   ={"ÈıÈşÉ¡É¢"};
static char PY_mb_sang[]  ={"É£É¤É¥"};
static char PY_mb_sao[]   ={"É¦É§É¨É©"};
static char PY_mb_se[]    ={"É«É¬Éª"};
static char PY_mb_sen[]   ={"É­"};
static char PY_mb_seng[]  ={"É®"};
static char PY_mb_sha[]   ={"É±É³É´É°É¯ÉµÉ¶É·ÏÃ"};
static char PY_mb_shai[]  ={"É¸É¹"};
static char PY_mb_shan[]  ={"É½É¾É¼ÉÀÉºÉ¿ÉÁÉÂÉÇÉ»ÉÈÉÆÉÉÉÃÉÅÉÄÕ¤"};
static char PY_mb_shang[] ={"ÉËÉÌÉÊÉÑÉÎÉÍÉÏÉĞ"};
static char PY_mb_shao[]  ={"ÉÓÉÒÉÕÉÔÉ×ÉÖÉØÉÙÉÛÉÜÉÚ"};
static char PY_mb_she[]   ={""};
static char PY_mb_shen[]  ={"ÉEEúåEğÉEéÉûåñÉòÉóÉôÉöÉõÉøÉ÷Ê²"};
static char PY_mb_sheng[] ={"ÉıÉúÉùÉE¤ÉûÉşÊ¡Ê¥Ê¢Ê£"};
static char PY_mb_shi[]   ={"³×Ê¬Ê§Ê¦Ê­Ê«Ê©Ê¨ÊªÊ®Ê¯Ê±Ê¶ÊµÊ°Ê´Ê³Ê·Ê¸Ê¹Ê¼Ê»ÊºÊ¿ÊÏÊÀÊËÊĞÊ¾Ê½ÊÂÊÌÊÆÊÓÊÔÊÎÊÒÊÑÊÃÊÇÊÁÊÊÊÅÊÍÊÈÊÄÊÉËÆ"};
static char PY_mb_shou[]  ={"ÊÕÊÖÊØÊ×ÊÙÊÜÊŞÊÛÊÚÊİ"};
static char PY_mb_shu[]   ={""};
static char PY_mb_shua[]  ={"Ë¢Ë£"};
static char PY_mb_shuai[] ={"Ë¥Ë¤Ë¦Ë§"};
static char PY_mb_shuan[] ={"Ë©Ë¨"};
static char PY_mb_shuang[]={"Ë«ËªË¬"};
static char PY_mb_shui[]  ={"Ë­Ë®Ë°Ë¯"};
static char PY_mb_shun[]  ={"Ë±Ë³Ë´Ë²"};
static char PY_mb_shuo[]  ={"ËµË¸Ë·Ë¶"};
static char PY_mb_si[]    ={"Ë¿Ë¾Ë½Ë¼Ë¹Ë»ËºËÀËÈËÄËÂËÅËÇËÃËÁ"};
static char PY_mb_song[]  ={"ËÉËËËÊËÏËÎËĞËÍËÌ"};
static char PY_mb_sou[]   ={"ËÔËÑËÒËÓ"};
static char PY_mb_su[]    ={"ËÕËÖË×ËßËàËØËÙËÚËÜËİËÛ"};
static char PY_mb_suan[]  ={""};
static char PY_mb_sui[]   ={""};
static char PY_mb_sun[]   ={""};
static char PY_mb_suo[]   ={""};
static char PY_mb_ta[]    ={"ËıËûËEúËşÌ¡Ì¢Ì¤Ì£"};
static char PY_mb_tai[]   ={"Ì¥Ì¨Ì§Ì¦Ì«Ì­Ì¬Ì©Ìª"};
static char PY_mb_tan[]   ={"Ì®Ì°Ì¯Ì²Ì±Ì³Ì¸ÌµÌ·Ì¶Ì´Ì¹Ì»ÌºÌ¾Ì¿Ì½Ì¼"};
static char PY_mb_tang[]  ={"ÌÀÌÆÌÃÌÄÌÁÌÂÌÅÌÇÌÈÌÊÌÉÌÌÌË"};
static char PY_mb_tao[]   ={"ÌÎÌĞÌÍÌÏÌÓÌÒÌÕÌÔÌÑÌÖÌ×"};
static char PY_mb_te[]    ={"ÌØ"};
static char PY_mb_teng[]  ={"ÌÛÌÚÌÜÌÙ"};
static char PY_mb_ti[]    ={""};
static char PY_mb_tian[]  ={""};
static char PY_mb_tiao[]  ={""};
static char PY_mb_tie[]   ={""};
static char PY_mb_ting[]  ={"ÌE¡ÌıÌşÍ¢Í¤Í¥Í£Í¦Í§"};
static char PY_mb_tong[]  ={"Í¨Í¬Í®Í©Í­Í¯ÍªÍ«Í³Í±Í°Í²Í´"};
static char PY_mb_tou[]   ={"ÍµÍ·Í¶Í¸"};
static char PY_mb_tu[]    ={"Í¹ÍºÍ»Í¼Í½Í¿Í¾ÍÀÍÁÍÂÍÃ"};
static char PY_mb_tuan[]  ={"ÍÄÍÅ"};
static char PY_mb_tui[]   ={"ÍÆÍÇÍÈÍËÍÉÍÊ"};
static char PY_mb_tun[]   ={"¶ÚÍÌÍÍÍÎ"};
static char PY_mb_tuo[]   ={"ÍĞÍÏÍÑÍÔÍÓÍÕÍÒÍ×ÍÖÍØÍÙ"};
static char PY_mb_wa[]    ={""};
static char PY_mb_wai[]   ={""};
static char PY_mb_wan[]   ={""};
static char PY_mb_wang[]  ={""};
static char PY_mb_wei[]   ={"Î£ÍşÎ¢Î¡ÎªÎ¤Î§Î¥Î¦Î¨Î©Î¬Î«Î°Î±Î²Î³Î­Î¯Î®ÎÀÎ´Î»Î¶Î·Î¸Î¾Î½Î¹Î¼ÎµÎ¿Îº"};
static char PY_mb_wen[]   ={"ÎÂÎÁÎÄÎÆÎÅÎÃÎÇÎÉÎÈÎÊ"};
static char PY_mb_weng[]  ={"ÎÌÎËÎÍ"};
static char PY_mb_wo[]    ={"ÎÎÎĞÎÑÎÏÎÒÎÖÎÔÎÕÎÓ"};
static char PY_mb_wu[]    ={""};
static char PY_mb_xi[]    ={"Ï¦Ï«Î÷ÎE£ÎôÎöÎùÏ¢ÎşÏ¤Ï§Ï©ÎøÎúÏ¬Ï¡ÏªÎıÏ¨ÎõÎûÏ¥Ï°Ï¯Ï®Ï±Ï­Ï´Ï²Ï·ÏµÏ¸Ï¶"};
static char PY_mb_xia[]   ={"ÏºÏ¹Ï»ÏÀÏ¿ÏÁÏ¾Ï½Ï¼ÏÂÏÅÏÄ"};
static char PY_mb_xian[]  ={"Ï³ÏÉÏÈÏËÏÆÏÇÏÊÏĞÏÒÏÍÏÌÏÑÏÏÏÎÏÓÏÔÏÕÏØÏÖÏßÏŞÏÜÏİÏÚÏÛÏ×ÏÙ"};
static char PY_mb_xiang[] ={""};
static char PY_mb_xiao[]  ={"ÏEûÏôÏõÏúÏöÏùÏıĞ¡ÏşĞ¢Ğ¤ÏøĞ§Ğ£Ğ¦Ğ¥"};
static char PY_mb_xie[]   ={"Ğ©Ğ¨ĞªĞ«Ğ­Ğ°Ğ²Ğ±Ğ³Ğ¯Ğ¬Ğ´Ğ¹ĞºĞ¶Ğ¼ĞµĞ»Ğ¸Ğ·"};
static char PY_mb_xin[]   ={"ĞÄĞÃĞ¾ĞÁĞÀĞ¿ĞÂĞ½ĞÅĞÆ"};
static char PY_mb_xing[]  ={"ĞËĞÇĞÊĞÉĞÈĞÌĞÏĞÎĞÍĞÑĞÓĞÕĞÒĞÔ"};
static char PY_mb_xiong[] ={"Ğ×ĞÖĞÙĞÚĞØĞÛĞÜ"};
static char PY_mb_xiu[]   ={""};
static char PY_mb_xu[]    ={""};
static char PY_mb_xuan[]  ={"ĞùĞûĞúĞşĞEıÑ¡Ñ¢Ñ¤Ñ£"};
static char PY_mb_xue[]   ={"Ï÷Ñ¥Ñ¦Ñ¨Ñ§Ñ©Ñª"};
static char PY_mb_xun[]   ={"Ñ«Ñ¬Ñ°Ñ²Ñ®Ñ±Ñ¯Ñ­ÑµÑ¶Ñ´Ñ¸Ñ·Ñ³"};
static char PY_mb_ya[]    ={"Ñ¾Ñ¹Ñ½ÑºÑ»Ñ¼ÑÀÑ¿ÑÁÑÂÑÄÑÃÑÆÑÅÑÇÑÈ"};
static char PY_mb_yan[]   ={""};
static char PY_mb_yang[]  ={""};
static char PY_mb_yao[]   ={"½ÄÑıÑEûÒ¢Ò¦Ò¤Ò¥Ò¡Ò£ÑşÒ§Ò¨Ò©ÒªÒ«Ô¿"};
static char PY_mb_ye[]    ={"Ò¬Ò­Ò¯Ò®Ò²Ò±Ò°ÒµÒ¶Ò·Ò³Ò¹Ò´ÒºÒ¸"};
static char PY_mb_yi[]    ={"Ò»ÒÁÒÂÒ½ÒÀÒ¿Ò¼Ò¾ÒÇÒÄÒÊÒËÒÌÒÈÒÆÒÅÒÃÒÉÒÍÒÒÒÑÒÔÒÓÒÏÒĞÒÎÒåÒÚÒäÒÕÒéÒàÒÙÒEÛÒÖÒEØÒ×ÒEèÒßÒæÒEûîİÒâÒçÒŞÒáÒãÒúîÜ"};
static char PY_mb_yin[]   ={"ÒòÒõÒöÒğÒñÒôÒóÒ÷ÒúÒùÒøÒEıÒûÒşÓ¡"};
static char PY_mb_ying[]  ={"Ó¦Ó¢Ó¤Ó§Ó£Ó¥Ó­Ó¯Ó«Ó¨Ó©ÓªÓ¬Ó®Ó±Ó°Ó³Ó²"};
static char PY_mb_yo[]    ={"Ó´"};
static char PY_mb_yong[]  ={"Ó¶ÓµÓ¸Ó¹ÓºÓ·ÓÀÓ½Ó¾ÓÂÓ¿ÓÁÓ¼Ó»ÓÃ"};
static char PY_mb_you[]   ={"ÓÅÓÇÓÄÓÆÓÈÓÉÓÌÓÊÓÍÓËÓÎÓÑÓĞÓÏÓÖÓÒÓ×ÓÓÓÕÓÔ"};
static char PY_mb_yu[]    ={"ÓØÓÙÓåÓÚÓèÓàÓÛÓãÓáÓéÓæÓçÓäÓâÓŞÓÜÓİÓßÓEûïEğÓEúïEñÔ¦ÓóÓıÓôÓEøÔ¡Ô¤ÓòÓûÓ÷Ô¢ÓùÔ£ÓöÓúÓşÔ¥"};
static char PY_mb_yuan[]  ={"Ô©Ô§Ô¨ÔªÔ±Ô°Ô«Ô­Ô²Ô¬Ô®ÔµÔ´Ô³Ô¯Ô¶Ô·Ô¹ÔºÔ¸"};
static char PY_mb_yue[]   ={"Ô»Ô¼ÔÂÔÀÔÃÔÄÔ¾ÔÁÔ½"};
static char PY_mb_yun[]   ={"ÔÆÔÈÔÇÔÅÔÊÔÉÔĞÔËÔÎÔÍÔÏÔÌ"};
static char PY_mb_za[]    ={"ÔÑÔÓÔÒÕ¦"};
static char PY_mb_zai[]   ={"ÔÖÔÕÔÔÔ×ÔØÔÙÔÚ×Ğ"};
static char PY_mb_zan[]   ={"ÔÛÔÜÔİÔŞ"};
static char PY_mb_zang[]  ={""};
static char PY_mb_zao[]   ={""};
static char PY_mb_ze[]    ={""};
static char PY_mb_zei[]   ={""};
static char PY_mb_zen[]   ={""};
static char PY_mb_zeng[]  ={""};
static char PY_mb_zha[]   ={""};
static char PY_mb_zhai[]  ={"Õ«ÕªÕ¬µÔÕ­Õ®Õ¯"};
static char PY_mb_zhan[]  ={"Õ´Õ±Õ³Õ²Õ°Õ¶Õ¹ÕµÕ¸Õ·Õ¼Õ½Õ»Õ¾ÕÀÕ¿Õº"};
static char PY_mb_zhang[] ={"³¤ÕÅÕÂÕÃÕÄÕÁÕÇÕÆÕÉÕÌÕÊÕÈÕÍÕËÕÏÕÎó¯"};
static char PY_mb_zhao[]  ={"ÕĞÕÑÕÒÕÓÕÙÕ×ÕÔÕÕÕÖÕØ×¦"};
static char PY_mb_zhe[]   ={"ÕÚÕÛÕÜÕİÕŞÕßÕàÕâÕãÕá×Å"};
static char PY_mb_zhen[]  ={"ÕEEEäÕæÕèÕåÕçÕéÕEúñûñóÕñÕòÕğÖ¡"};
static char PY_mb_zheng[] ={"ÕùÕ÷ÕúÕõÕøÕöÕôÕEûÕıÖ¤Ö£ÕşÖ¢"};
static char PY_mb_zhi[]   ={"Ö®Ö§Ö­Ö¥Ö¨Ö¦ÖªÖ¯Ö«Ö¬Ö©Ö´Ö¶Ö±ÖµÖ°Ö²Ö³Ö¹Ö»Ö¼Ö·Ö½Ö¸ÖºÖÁÖ¾ÖÆÖÄÖÎÖËÖÊÖÅÖ¿ÖÈÖÂÖÀÖÌÖÏÖÇÖÍÖÉÖÃ"};
static char PY_mb_zhong[] ={"ÖĞÖÒÖÕÖÑÖÓÖÔÖ×ÖÖÖÙÖÚÖØ"};
static char PY_mb_zhou[]  ={""};
static char PY_mb_zhu[]   ={"ÖEEEéÖûòúòEñÖòÖğÖ÷ÖôÖóÖöÖõ×¡Öú×¢ÖE¤Öù×£ÖøÖûÖşÖı"};
static char PY_mb_zhua[]  ={"×¥"};
static char PY_mb_zhuai[] ={"×§"};
static char PY_mb_zhuan[] ={"×¨×©×ª×«×­"};
static char PY_mb_zhuang[]={"×±×¯×®×°×³×´´±×²"};
static char PY_mb_zhui[]  ={"×·×µ×¶×¹×º×¸"};
static char PY_mb_zhun[]  ={"×»×¼"};
static char PY_mb_zhuo[]  ={"×¿×¾×½×À×Æ×Â×Ç×Ã×Ä×Á"};
static char PY_mb_zi[]    ={"×Î×È×É×Ë×Ê×Í×Ì×Ñ×Ó×Ï×Ò×Ö×Ô×Õ"};
static char PY_mb_zong[]  ={"×Ú×Û×Ø×Ù×××Ü×İ"};
static char PY_mb_zou[]   ={""};
static char PY_mb_zu[]    ={""};
static char PY_mb_zuan[]  ={""};
static char PY_mb_zui[]   ={""};
static char PY_mb_zun[]   ={""};
static char PY_mb_zuo[]   ={""};
static char PY_mb_space[] ={""};

/*"Æ´ÒôÊäÈE¨²éÑ¯ÂEE¶ş¼¶×ÖÄ¸Ë÷Òı±Eindex)"*/
static struct PY_index  PY_index_a[]={{"",PY_mb_a},
{"i",PY_mb_ai},
{"n",PY_mb_an},
{"ng",PY_mb_ang},
{"o",PY_mb_ao}};
static struct PY_index  PY_index_b[]={{"a",PY_mb_ba},
{"ai",PY_mb_bai},
{"an",PY_mb_ban},
{"ang",PY_mb_bang},
{"ao",PY_mb_bao},
{"ei",PY_mb_bei},
{"en",PY_mb_ben},
{"eng",PY_mb_beng},
{"i",PY_mb_bi},
{"ian",PY_mb_bian},
{"iao",PY_mb_biao},
{"ie",PY_mb_bie},
{"in",PY_mb_bin},
{"ing",PY_mb_bing},
{"o",PY_mb_bo},
{"u",PY_mb_bu}};
static struct PY_index  PY_index_c[]={{"a",PY_mb_ca},
{"ai",PY_mb_cai},
{"an",PY_mb_can},
{"ang",PY_mb_cang},
{"ao",PY_mb_cao},
{"e",PY_mb_ce},
{"eng",PY_mb_ceng},
{"ha",PY_mb_cha},
{"hai",PY_mb_chai},
{"han",PY_mb_chan},
{"hang",PY_mb_chang},
{"hao",PY_mb_chao},
{"he",PY_mb_che},
{"hen",PY_mb_chen},
{"heng",PY_mb_cheng},
{"hi",PY_mb_chi},
{"hong",PY_mb_chong},
{"hou",PY_mb_chou},
{"hu",PY_mb_chu},
{"huai",PY_mb_chuai},
{"huan",PY_mb_chuan},
{"huang",PY_mb_chuang},
{"hui",PY_mb_chui},
{"hun",PY_mb_chun},
{"huo",PY_mb_chuo},
{"i",PY_mb_ci},
{"ong",PY_mb_cong},
{"ou",PY_mb_cou},
{"u",PY_mb_cu},
{"uan",PY_mb_cuan},
{"ui",PY_mb_cui},
{"un",PY_mb_cun},
{"uo",PY_mb_cuo}};
static struct PY_index  PY_index_d[]={{"a",PY_mb_da},
{"ai",PY_mb_dai},
{"an",PY_mb_dan},
{"ang",PY_mb_dang},
{"ao",PY_mb_dao},
{"e",PY_mb_de},
{"eng",PY_mb_deng},
{"i",PY_mb_di},
{"ian",PY_mb_dian},
{"iao",PY_mb_diao},
{"ie",PY_mb_die},
{"ing",PY_mb_ding},
{"iu",PY_mb_diu},
{"ong",PY_mb_dong},
{"ou",PY_mb_dou},
{"u",PY_mb_du},
{"uan",PY_mb_duan},
{"ui",PY_mb_dui},
{"un",PY_mb_dun},
{"uo",PY_mb_duo}};
static struct PY_index  PY_index_e[]={{"",PY_mb_e},
{"n",PY_mb_en},
{"r",PY_mb_er}};
static struct PY_index  PY_index_f[]={{"a",PY_mb_fa},
{"an",PY_mb_fan},
{"ang",PY_mb_fang},
{"ei",PY_mb_fei},
{"en",PY_mb_fen},
{"eng",PY_mb_feng},
{"o",PY_mb_fo},
{"ou",PY_mb_fou},
{"u",PY_mb_fu}};
static struct PY_index  PY_index_g[]={{"a",PY_mb_ga},
{"ai",PY_mb_gai},
{"an",PY_mb_gan},
{"ang",PY_mb_gang},
{"ao",PY_mb_gao},
{"e",PY_mb_ge},
{"ei",PY_mb_gei},
{"en",PY_mb_gen},
{"eng",PY_mb_geng},
{"ong",PY_mb_gong},
{"ou",PY_mb_gou},
{"u",PY_mb_gu},
{"ua",PY_mb_gua},
{"uai",PY_mb_guai},
{"uan",PY_mb_guan},
{"uang",PY_mb_guang},
{"ui",PY_mb_gui},
{"un",PY_mb_gun},
{"uo",PY_mb_guo}};
static struct PY_index  PY_index_h[]={{"a",PY_mb_ha},
{"ai",PY_mb_hai},
{"an",PY_mb_han},
{"ang",PY_mb_hang},
{"ao",PY_mb_hao},
{"e",PY_mb_he},
{"ei",PY_mb_hei},
{"en",PY_mb_hen},
{"eng",PY_mb_heng},
{"ong",PY_mb_hong},
{"ou",PY_mb_hou},
{"u",PY_mb_hu},
{"ua",PY_mb_hua},
{"uai",PY_mb_huai},
{"uan",PY_mb_huan},
{"uang ",PY_mb_huang},
{"ui",PY_mb_hui},
{"un",PY_mb_hun},
{"uo",PY_mb_huo}};
static struct PY_index  PY_index_i[]={{"",PY_mb_space}};
static struct PY_index  PY_index_j[]={{"i",PY_mb_ji},
{"ia",PY_mb_jia},
{"ian",PY_mb_jian},
{"iang",PY_mb_jiang},
{"iao",PY_mb_jiao},
{"ie",PY_mb_jie},
{"in",PY_mb_jin},
{"ing",PY_mb_jing},
{"iong",PY_mb_jiong},
{"iu",PY_mb_jiu},
{"u",PY_mb_ju},
{"uan",PY_mb_juan},
{"ue",PY_mb_jue},
{"un",PY_mb_jun}};
static struct PY_index  PY_index_k[]={{"a",PY_mb_ka},
{"ai",PY_mb_kai},
{"an",PY_mb_kan},
{"ang",PY_mb_kang},
{"ao",PY_mb_kao},
{"e",PY_mb_ke},
{"en",PY_mb_ken},
{"eng",PY_mb_keng},
{"ong",PY_mb_kong},
{"ou",PY_mb_kou},
{"u",PY_mb_ku},
{"ua",PY_mb_kua},
{"uai",PY_mb_kuai},
{"uan",PY_mb_kuan},
{"uang",PY_mb_kuang},
{"ui",PY_mb_kui},
{"un",PY_mb_kun},
{"uo",PY_mb_kuo}};
static struct PY_index  PY_index_l[]={{"a",PY_mb_la},
{"ai",PY_mb_lai},
{"an",PY_mb_lan},
{"ang",PY_mb_lang},
{"ao",PY_mb_lao},
{"e",PY_mb_le},
{"ei",PY_mb_lei},
{"eng",PY_mb_leng},
{"i",PY_mb_li},
{"ian",PY_mb_lian},
{"iang",PY_mb_liang},
{"iao",PY_mb_liao},
{"ie",PY_mb_lie},
{"in",PY_mb_lin},
{"ing",PY_mb_ling},
{"iu",PY_mb_liu},
{"ong",PY_mb_long},
{"ou",PY_mb_lou},
{"u",PY_mb_lu},
{"uan",PY_mb_luan},
{"ue",PY_mb_lue},
{"un",PY_mb_lun},
{"uo",PY_mb_luo},
{"v",PY_mb_lv}};
static struct PY_index  PY_index_m[]={{"a",PY_mb_ma},
{"ai",PY_mb_mai},
{"an",PY_mb_man},
{"ang",PY_mb_mang},
{"ao",PY_mb_mao},
{"e",PY_mb_me},
{"ei",PY_mb_mei},
{"en",PY_mb_men},
{"eng",PY_mb_meng},
{"i",PY_mb_mi},
{"ian",PY_mb_mian},
{"iao",PY_mb_miao},
{"ie",PY_mb_mie},
{"in",PY_mb_min},
{"ing",PY_mb_ming},
{"iu",PY_mb_miu},
{"o",PY_mb_mo},
{"ou",PY_mb_mou},
{"u",PY_mb_mu}};
static struct PY_index  PY_index_n[]={{"a",PY_mb_na},
{"ai",PY_mb_nai},
{"an",PY_mb_nan},
{"ang",PY_mb_nang},
{"ao",PY_mb_nao},
{"e",PY_mb_ne},
{"ei",PY_mb_nei},
{"en",PY_mb_nen},
{"eng",PY_mb_neng},
{"i",PY_mb_ni},
{"ian",PY_mb_nian},
{"iang",PY_mb_niang},
{"iao",PY_mb_niao},
{"ie",PY_mb_nie},
{"in",PY_mb_nin},
{"ing",PY_mb_ning},
{"iu",PY_mb_niu},
{"ong",PY_mb_nong},
{"u",PY_mb_nu},
{"uan",PY_mb_nuan},
{"ue",PY_mb_nue},
{"uo",PY_mb_nuo},
{"v",PY_mb_nv}};
static struct PY_index  PY_index_o[]={{"",PY_mb_o},
{"u",PY_mb_ou}};
static struct PY_index  PY_index_p[]={{"a",PY_mb_pa},
{"ai",PY_mb_pai},
{"an",PY_mb_pan},
{"ang",PY_mb_pang},
{"ao",PY_mb_pao},
{"ei",PY_mb_pei},
{"en",PY_mb_pen},
{"eng",PY_mb_peng},
{"i",PY_mb_pi},
{"ian",PY_mb_pian},
{"iao",PY_mb_piao},
{"ie",PY_mb_pie},
{"in",PY_mb_pin},
{"ing",PY_mb_ping},
{"o",PY_mb_po},
{"ou",PY_mb_pou},
{"u",PY_mb_pu}};
static struct PY_index  PY_index_q[]={{"i",PY_mb_qi},
{"ia",PY_mb_qia},
{"ian",PY_mb_qian},
{"iang",PY_mb_qiang},
{"iao",PY_mb_qiao},
{"ie",PY_mb_qie},
{"in",PY_mb_qin},
{"ing",PY_mb_qing},
{"iong",PY_mb_qiong},
{"iu",PY_mb_qiu},
{"u",PY_mb_qu},
{"uan",PY_mb_quan},
{"ue",PY_mb_que},
{"un",PY_mb_qun}};
static struct PY_index  PY_index_r[]={{"an",PY_mb_ran},
{"ang",PY_mb_rang},
{"ao",PY_mb_rao},
{"e",PY_mb_re},
{"en",PY_mb_ren},
{"eng",PY_mb_reng},
{"i",PY_mb_ri},
{"ong",PY_mb_rong},
{"ou",PY_mb_rou},
{"u",PY_mb_ru},
{"uan",PY_mb_ruan},
{"ui",PY_mb_rui},
{"un",PY_mb_run},
{"uo",PY_mb_ruo}};
static struct PY_index  PY_index_s[]={{"a",PY_mb_sa},
{"ai",PY_mb_sai},
{"an",PY_mb_san},
{"ang",PY_mb_sang},
{"ao",PY_mb_sao},
{"e",PY_mb_se},
{"en",PY_mb_sen},
{"eng",PY_mb_seng},
{"ha",PY_mb_sha},
{"hai",PY_mb_shai},
{"han",PY_mb_shan},
{"hang ",PY_mb_shang},
{"hao",PY_mb_shao},
{"he",PY_mb_she},
{"hen",PY_mb_shen},
{"heng",PY_mb_sheng},
{"hi",PY_mb_shi},
{"hou",PY_mb_shou},
{"hu",PY_mb_shu},
{"hua",PY_mb_shua},
{"huai",PY_mb_shuai},
{"huan",PY_mb_shuan},
{"huang",PY_mb_shuang},
{"hui",PY_mb_shui},
{"hun",PY_mb_shun},
{"huo",PY_mb_shuo},
{"i",PY_mb_si},
{"ong",PY_mb_song},
{"ou",PY_mb_sou},
{"u",PY_mb_su},
{"uan",PY_mb_suan},
{"ui",PY_mb_sui},
{"un",PY_mb_sun},
{"uo",PY_mb_suo}};
static struct PY_index  PY_index_t[]={{"a",PY_mb_ta},
{"ai",PY_mb_tai},
{"an",PY_mb_tan},
{"ang",PY_mb_tang},
{"ao",PY_mb_tao},
{"e",PY_mb_te},
{"eng",PY_mb_teng},
{"i",PY_mb_ti},
{"ian",PY_mb_tian},
{"iao",PY_mb_tiao},
{"ie",PY_mb_tie},
{"ing",PY_mb_ting},
{"ong",PY_mb_tong},
{"ou",PY_mb_tou},
{"u",PY_mb_tu},
{"uan",PY_mb_tuan},
{"ui",PY_mb_tui},
{"un",PY_mb_tun},
{"uo",PY_mb_tuo}};
static struct PY_index  PY_index_u[]={{"",PY_mb_space}};
static struct PY_index  PY_index_v[]={{"",PY_mb_space}};
static struct PY_index  PY_index_w[]={{"a",PY_mb_wa},
{"ai",PY_mb_wai},
{"an",PY_mb_wan},
{"ang",PY_mb_wang},
{"ei",PY_mb_wei},
{"en",PY_mb_wen},
{"eng",PY_mb_weng},
{"o",PY_mb_wo},
{"u",PY_mb_wu}};
static struct PY_index PY_index_x[]={{"i",PY_mb_xi},
{"ia",PY_mb_xia},
{"ian",PY_mb_xian},
{"iang",PY_mb_xiang},
{"iao",PY_mb_xiao},
{"ie",PY_mb_xie},
{"in",PY_mb_xin},
{"ing",PY_mb_xing},
{"iong",PY_mb_xiong},
{"iu",PY_mb_xiu},
{"u",PY_mb_xu},
{"uan",PY_mb_xuan},
{"ue",PY_mb_xue},
{"un",PY_mb_xun}};
static struct PY_index PY_index_y[]={{"a",PY_mb_ya},
{"an",PY_mb_yan},
{"ang",PY_mb_yang},
{"ao",PY_mb_yao},
{"e",PY_mb_ye},
{"i",PY_mb_yi},
{"in",PY_mb_yin},
{"ing",PY_mb_ying},
{"o",PY_mb_yo},
{"ong",PY_mb_yong},
{"ou",PY_mb_you},
{"u",PY_mb_yu},
{"uan",PY_mb_yuan},
{"ue",PY_mb_yue},
{"un",PY_mb_yun}};
static struct PY_index PY_index_z[]={{"a",PY_mb_za},
{"ai",PY_mb_zai},
{"an",PY_mb_zan},
{"ang",PY_mb_zang},
{"ao",PY_mb_zao},
{"e",PY_mb_ze},
{"ei",PY_mb_zei},
{"en",PY_mb_zen},
{"eng",PY_mb_zeng},
{"ha",PY_mb_zha},
{"hai",PY_mb_zhai},
{"han",PY_mb_zhan},
{"hang",PY_mb_zhang},
{"hao",PY_mb_zhao},
{"he",PY_mb_zhe},
{"hen",PY_mb_zhen},
{"heng",PY_mb_zheng},
{"hi",PY_mb_zhi},
{"hong",PY_mb_zhong},
{"hou",PY_mb_zhou},
{"hu",PY_mb_zhu},
{"hua",PY_mb_zhua},
{"huai",PY_mb_zhuai},
{"huan",PY_mb_zhuan},
{"huang",PY_mb_zhuang},
{"hui",PY_mb_zhui},
{"hun",PY_mb_zhun},
{"huo",PY_mb_zhuo},
{"i",PY_mb_zi},
{"ong",PY_mb_zong},
{"ou",PY_mb_zou},
{"u",PY_mb_zu},
{"uan",PY_mb_zuan},
{"ui",PY_mb_zui},
{"un",PY_mb_zun},
{"uo",PY_mb_zuo}};

static struct PY_index  PY_index_end[]={{"",PY_mb_space}};

static const int PY_index_headsize[]={
	5,
	16,
	33,
	20,
	3,
	9,
	19,
	19,
	0,
	14,
	18,
	24,
	19,
	23,
	2,
	17,
	14,
	14,
	34,
	19,
	0,
	0,
	9,
	14,
	15,
	36
};
/* E*/
static struct PY_index  *PY_index_headletter[]={	PY_index_a,
PY_index_b,
PY_index_c,
PY_index_d,
PY_index_e,
PY_index_f,
PY_index_g,
PY_index_h,
PY_index_i,
PY_index_j,
PY_index_k,
PY_index_l,
PY_index_m,
PY_index_n,
PY_index_o,
PY_index_p,
PY_index_q,
PY_index_r,
PY_index_s,
PY_index_t,
PY_index_u,
PY_index_v,
PY_index_w,
PY_index_x,
PY_index_y,
PY_index_z,
PY_index_end}; 

enum InputStatus
{
	eInputEng = 0,
	eInputChi,
	eSelPY,
	eSelHZ,
	eInputNum
};
class JGBKFont;
class JInputSystem
{
private:
	JGBKFont* mBitmapFont12;
	bool mIsInputActive;

	float mTimer;
	//store input STRING
	char  mInPut[512];
	char * mpInput;
	//store input PY
	char  mPY[16];
	//pointer to the HZ select string.
	char *mHZ;
	//input system status.
	InputStatus mStatus;
	//PY select
	bool mEnablePYSel;
	int mPYShowFirstIndex;
	int mPYSelIndex;
	int mPYSelTableSize;

	//HZ select
	int mHZShowFirstIndex;
	int mHZSelIndex;
	int mHZSelTableSize;
	bool mIsHZ_H;

	bool GetInputKey(int& a, int& b, int& c);
	char* GetChinese(char* str);
	int GetNexPYIndex(char* str, PY_index* &py_index);
	void printf12(char* str,float x, float y, float scale=1.0f, PIXEL_TYPE color=ARGB(255,255,255,255),int type=JGETEXT_LEFT);
	int  strlen12( char* buff, float scale=1.0f);


public:
	static JInputSystem* m_pJInputSystem;
	static JInputSystem * GetInstance();
	static void Destory();
	JInputSystem(void);
	virtual ~JInputSystem(void);

	//Active flag
	void EnableInputMode(char *buf){buf[0]=0;mpInput = buf; mIsInputActive=true;}
	void DisableInputMode(){mpInput = NULL; mIsInputActive=false;}
	//void SetInputActive(bool f){mIsInputActive=f;}
	bool GetIsInputActive(){return mIsInputActive;}

	JGBKFont* GetFont12(){return mBitmapFont12;}

	//Update
	void Update();
	void UpdateInputEng();
	void UpdateInputChi();
	void UpdateSelPY();
	void UpdateSelHZ();
	void UpdateSelHZ_H();
	void UpdateInputNum();

	void Draw();
	void DrawInputString(float x,float y);
	void DrawStr1(char* str, float x, float y, u32 color=ARGB(255,0,0,0));
	void DrawStatus(float x,float y);
	void DrawPYInput(float x,float y);
	void DrawPYSel(float x,float y);
	void DrawHZSel(float x,float y);
	void DrawHZSel_H(float x,float y);
	void DrawInputHelp(float x, float y);

	char * GetInputString(){return mInPut;}
};
#endif

