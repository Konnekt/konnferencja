/*
  KONNferencja - obs³uga konferencji dla protoko³u Gadu-Gadu

  (c)2003 Rafa³ Lindemann / www.stamina.eu.org / www.konnekt.info

  Obsluga API Konnekta

  Kod udostêpniany na licencji GPL, której treœæ powinna byæ dostarczona
  razem z tym kodem.
  Licensed under GPL.

*/

#include "stdafx.h"
#include <konnekt/konnferencja.h>
#include "konnferencja_main.h"

using namespace konnfer;

namespace konnfer {
    gg_session * session = 0;
    std::deque<group_holder> groups;
};

//icon magic function by SIJA
std::string Icon32( int ico )
{
     char txt[32];
     std::string buff;
     
     sprintf( txt, "reg://IML32/%i.ICON", ico );     
     buff = AP_IMGURL;
     buff += txt;

     return buff;
}

int __stdcall DllMain(void * hinstDLL, unsigned long fdwReason, void * lpvReserved)
{
        return true;
}
int Init() {
    // Trzeba utworzyæ obiekty dla istniej¹cych kontaktów
    return 1;
}

int DeInit() {
    while (groups.size())
        delete *groups.front();
    return 1;
}

int IStart() {
    int count = ICMessage(IMC_CNT_COUNT);
    for (int i=1; i<count;i++)
        if (GETCNTI(i , CNT_NET)==konnfer::net)
            createGroupObject(Ctrl->DTgetID(DTCNT , i));
    /* Próbujemy zarejestrowaæ siê w gg.dll */
    IMessage(IM_GG_REGISTERHANDLER,NET_GG,IMT_PROTOCOL,GGER_FIRSTLOOP|GGER_EVENT|GGER_LOGOUT);
    if (Ctrl->getError()==IMERROR_UNSUPPORTEDMSG) {// skoro nikt nie obs³u¿y³ tej wiadomoœci, znaczy ¿e gg.dll jest wy³¹czone
        ICMessage(IMI_INFORM , (int)"Wtyczka Konnferencja nie bêdzie dzia³aæ bez w³¹czonej wtyczki GaduGadu!");
        return 0;
    }
    return 1;
}
int IEnd() {
  return 1;
}

#define CFGSETCOL(i,t,d) {sSETCOL sc;sc.id=(i);sc.type=(t);sc.def=(int)(d);ICMessage(IMC_CFG_SETCOL,(int)&sc);}
int ISetCols() {
	//skolima ADD
	// definiujemy wartoœæ dla opcji
    SetColumn (DTCFG, KONNF_OPCJE_IGNORETEXT, DT_CT_STR, "Konferencja {Display} nie zosta³a autoryzowana.Jeœli chcesz rozmawiaæ, daj mi najpierw znaæ prywatnie.", "Konnferencja/IgnoreText");
    SetColumn (DTCFG, KONNF_OPCJE_RESPONDTOWHOM, DT_CT_INT, 0, "Konnferencja/RespondToWhom");
    SetColumn (DTCFG, KONNF_OPCJE_RESPOND, DT_CT_INT, 0, "Konnferencja/Respond");
    SetColumn (DTCFG, KONNF_OPCJE_IGNOREBYDEFAULT, DT_CT_INT, 1, "Konnferencja/IgrnoreByDefault");
	SetColumn (DTCNT, KONNF_OPCJE_KONTAKT_TIMESTAMP, DT_CT_INT, 0, "Konnferencja/LastMsgTimestamp" ); //niewidoczne dla usera
	SetColumn (DTCFG, KONNF_OPCJE_IGNOREIF, DT_CT_INT, 0, "Konnferencja/IgnoreIfUnknown");
    // koñczymy dzia³anie funkcji
	//end skolima ADD
  return 1;
}

int IPrepare() {
    IconRegister(IML_16 , Ico::group_active , Ctrl->hDll() , 20000);
    IconRegister(IML_16 , Ico::group_inactive , Ctrl->hDll() , 20001);
    IconRegister(IML_16 , Ico::group_msg , Ctrl->hDll() , 20002);
    IconRegister(IML_16 , Ico::group_show , Ctrl->hDll() , 20003);
	//skolima ADD LINE
	IconRegister(IML_32 , KONNF_IKONA_32 , Ctrl->hDll() , 20004);
    IconRegister(IML_16 , UIIcon(IT_LOGO , konnfer::net , 0 , 0) , Ctrl->hDll() , 20000);
    UIActionInsert(IMIG_CNT , konnfer::Action::start_conference , 1 , ACTR_INIT , "Zacznij konferencjê" , Ico::group_active);
    UIActionInsert(IMIG_MSGTB , konnfer::Action::show_recipients , 1 , ACTR_INIT , "Wypisz rozmówców" , Ico::group_show);
    UIActionInsert(IMIG_NFO_DETAILS , konnfer::Action::nfo_dummy , 0 , ACTS_HIDDEN | ACTR_INIT , "");

	//skolima ADD
	// dodajemy pozycje w okienku konfiguracyjnym
	UIGroupAdd (IMIG_GGCFG_USER, KONNF_OPCJEID_GRUPA, 0, "KONNferencja", Ico::group_active);
	
	char bufor [1024] = "";
	wsprintf(bufor, "<br/>Copyright © 2004-05 <b>Stamina</b><br/>Autor i opiekun wtyczki: <b>Rafa³ \"Hao\" Lindemann</b><br/>Modyfikacja: <b>Leszek \"Skolima\" Ciesielski</b><br/><br/>Wiêcej informacji i Ÿród³a na stronie projektu http://kplugins.net/<br/>Data kompilacji: <b>%s</b> @ <b>%s</b>", __TIME__, __DATE__);
	UIActionCfgAddPluginInfoBox2(KONNF_OPCJEID_GRUPA, "KONNferencja rozszerza mo¿liwoœci wtyczki Gadu-Gadu™ - dodaje w niej obs³ugê rozmów konferencyjnych. Konferencje pojawiaj¹ siê jako osobne kontakty, dziêki czemu mog¹ byæ zapisane do póŸniejszego u¿ycia.", bufor, Icon32( KONNF_IKONA_32 ).c_str(),-4);
	//nowa grupa
	UIActionAdd(KONNF_OPCJEID_GRUPA , 0 , ACTT_GROUP , "Opcje ignorowania");
	//checkbox
	UIActionCfgAdd (KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREBYDEFAULT, ACTT_CHECK|ACTR_SHOW, 
		"Domyœlnie ignoruj nieznane konferencje :"
		, KONNF_OPCJE_IGNOREBYDEFAULT);
	//
	char res[250];
	//dropdown box 
	sprintf (res, " %s" CFGVALUE "1" "\n %s" CFGVALUE "0",
		"zawsze", "gdy nie znam któregoœ rozmówcy");
	UIActionCfgAdd (KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF,
		ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_NOICON ,
		res, KONNF_OPCJE_IGNOREIF, 0, 0, 260);
	//kolejny checkbox
	UIActionCfgAdd (KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPOND, ACTT_CHECK, 
		"Odpisuj na wiadomoœci od ignorowanych konferencji : "
		, KONNF_OPCJE_RESPOND);
	// definiujemy pole combo 
    sprintf (res, " %s" CFGVALUE "1" "\n %s" CFGVALUE "0",
		"do ca³ej konferencji", "tylko do nadawcy");
	UIActionCfgAdd (KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM,
		ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_NOICON ,
		res, KONNF_OPCJE_RESPONDTOWHOM, 0, 0, 260);
	//odpowiedŸ dla ignorowanych
	UIActionAdd (KONNF_OPCJEID_GRUPA, 0, ACTT_COMMENT, 
		"Na ignorowane wiadomoœæi odpowiedz tekstem : "
		, 0, 0);
	UIActionCfgAdd ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNORETEXT, ACTT_TEXT,
		"" CFGTIP "Rozpoznawane zmienne : \n{Display}", KONNF_OPCJE_IGNORETEXT );
	//grupê trzeba zamkn¹æ
	UIActionAdd(KONNF_OPCJEID_GRUPA , 0 , ACTT_GROUPEND);
	//end skolima ADD

    return 1;
}

/*int ActionCfgProc(sUIActionNotify_base * anBase) {
  sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
  switch (anBase->act.id & ~IMIB_CFG) {
  }
  return 0;
}*/

ActionProc(sUIActionNotify_base * anBase) {
    sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
//    if ((anBase->act.id & IMIB_) == IMIB_CFG) return ActionCfgProc(anBase);
    switch (anBase->act.id) {
        case Action::nfo_dummy:
            if (ACTN_CREATE) {
                // Pewne rzeczy wypada³oby ukryæ
                bool isConfer = GETCNTI(anBase->act.cnt , CNT_NET) == konnfer::net;
                //UIActionSetStatus(sUIAction(IMIG_NFO_DETAILS , IMIB_CNT | CNT_NET) , isConfer?ACTS_HIDDEN:0 , ACTS_HIDDEN);
                // Lepiej, ¿eby u¿ytkownik nie zmienia³ UID'a...
                UIActionSetStatus(sUIAction(IMIG_NFO_DETAILS , IMIB_CNT | CNT_UID) , isConfer?ACTS_DISABLED:0 , ACTS_DISABLED);
            }
            return 0;
        case Action::show_recipients:
            if (anBase->code == ACTN_CREATE) {
                // Pewne rzeczy wypada³oby ukryæ
                bool isConfer = GETCNTI(anBase->act.cnt , CNT_NET) == konnfer::net;
                UIActionSetStatus(anBase->act , isConfer?0:ACTS_HIDDEN , ACTS_HIDDEN);
            } else if (anBase->code == ACTN_ACTION) {
                /*
                Informacjê wrzucamy jako wiadomoœæ MT_QUICKEVENT
                do okna z konferencj¹...
                */
                cMessage m;
                // wysy³amy do okna, w którym zosta³ naciœniêty
                // przycisk. Aby wys³aæ, potrzebujemy UID, pobieramy wiêc go :)
                string uid = GETCNTC(anBase->act.cnt , CNT_UID);
                // Gdy nie wiemy który UID ustawiæ, najlepiej ustawiæ oba :)
                m.toUid = m.fromUid = (char*)uid.c_str();
                m.net = konnfer::net;
                m.type = MT_QUICKEVENT;
                std::stringstream body;// strumieniem wygodniej póŸniej konwertowaæ dane
                body << "W konferencji uczestnicz¹:";
                groupItems gi;
                int net = getGroupUIDs(uid , gi);
                for (groupItems_it it = gi.begin(); it != gi.end(); it++) {
                    body << "\n";
                    int cnt = ICMessage(IMC_FINDCONTACT , net , (int)it->c_str());
                    if (cnt != -1) {
                        body << GETCNTC(cnt , CNT_DISPLAY)
                            << " [" << *it << "]";
                    }
                    else body << *it;
                }
                string body2 = body.str();
                m.body = (char*)body2.c_str();
                // Koniecznie trzeba ustawiæ, aby to UI obs³u¿y³o t¹ wiadomoœæ!
                m.flag = MF_HANDLEDBYUI;
                ICMessage(IMC_NEWMESSAGE , (int)&m);
                // UI obs³uguje QUICKEVENT ju¿ w IM_MSG_RCV. Nie ma wiêc potrzeby
                // wywo³ywania IMC_MESSAGE_QUEUE
            }
            return 0;
        case Action::start_conference: {
            switch (anBase->code) {
                case ACTN_CREATE:{
                    // Pewne rzeczy wypada³oby ukryæ
                    bool isConfer = GETCNTI(anBase->act.cnt , CNT_NET) == konnfer::net;
                    UIActionSetStatus(sUIAction(IMIG_CNT , IMIA_CNT_IGNORE) , isConfer?ACTS_DISABLED:0 , ACTS_DISABLED);
                    // Sprawdzamy, czy s¹ warunki, aby wyœwietliæ pozycjê w menu
                    // Konferencja ma sens tylko przy kilku kontaktach
                    if (anBase->act.cnt != -1) {
                        UIActionSetStatus(anBase->act , ACTS_HIDDEN , ACTS_HIDDEN);
                        return 0;
                    }
                    // I to kontaktach tej samej sieci... sieci GG jak na razie
                    int selCount = ICMessage(IMI_LST_SELCOUNT);
                    for (int i = 0; i < selCount; i++) {
                        if (GETCNTI(ICMessage(IMI_LST_GETSELPOS , i) , CNT_NET)!=NET_GG) {
                            UIActionSetStatus(anBase->act , ACTS_HIDDEN , ACTS_HIDDEN);
                            return 0;
                        }
                    }
                    // Skoro nie ma nic przeciwko, pokazujemy element
                    UIActionSetStatus(anBase->act , 0 , ACTS_HIDDEN);
                    return 0;}
                case ACTN_ACTION: {
                    /* Menu zosta³o wybrane... Zak³adamy, ¿e skoro zosta³o
                     naciœniête - by³o widoczne, a skoro by³o widoczne,
                     to wszystkie wymagane warunki zosta³y spe³nione...
                     i mo¿emy tworzyæ obiekt listy
                     Najpierw tworzymy kontakt o UIDzie z list¹ kontaktów,
                     a Konnekt ju¿ sam wywo³a IM_CNT_ADD co automatycznie
                     zarejestruje obiekt u nas...
                    */
                    //std::string list , display = "K:";
                    uin_t uins[100];
                    int selCount = ICMessage(IMI_LST_SELCOUNT);
                    if (selCount > 100) selCount = 100;
                    for (int i = 0; i < selCount; i++) {
                        //if (i) display+=",";
                        int cnt = ICMessage(IMI_LST_GETSELPOS , i);
                        uins[i]=GETCNTI(cnt , CNT_UID);
                        //display+=GETCNTC(cnt , CNT_DISPLAY);
                    }                
                    group_gg * gr;
                    // Tylko, gdy nie ma ju¿ tego samego...
                    groups_it gr_it = std::find(groups.begin() , groups.end() , groupContents_gg(selCount , uins));
                    if (gr_it == groups.end())
                        gr = new group_gg(selCount , uins , "" , true);
                    else
                        gr = static_cast<group_gg*>(**gr_it);
                    // Po utworzeniu kontaktu, otworzymy okno rozmowy...
                    // Udamy wiêc ¿e nacisnêliœmy "Wyœlij wiadomoœæ"
                    UIActionCall(&sUIActionNotify_2params(sUIAction(IMIG_CNT , IMIA_CNT_MSGOPEN , gr->getCnt()) , ACTN_ACTION , 0 , 0));
                    return 0;}
            }
            break;}
	//skolima ADD
		case KONNF_OPCJE_RESPOND :
			{
				ACTIONONLY( an );
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM,
					(UIActionGetStatus( sUIAction( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNORETEXT,
					(UIActionGetStatus( sUIAction( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNORETEXT ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
			}
			break;
		case KONNF_OPCJE_IGNOREBYDEFAULT :
			{
				if(anBase->code == ACTN_SHOW)//UI 'Ustawienia' bêdzie zaraz pokazywane
				{
					//dirty bugfix

				//ustawiam status zgodny z wpisem konfiguracji
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM,
					(GETINT(KONNF_OPCJE_RESPOND)) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM,
					(UIActionGetStatus( sUIAction( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny, czyli z powrotem w³aœciwy
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM,
					(UIActionGetStatus( sUIAction( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_RESPONDTOWHOM ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status zgodny z wpisem konfiguracji
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNORETEXT,
					(GETINT(KONNF_OPCJE_RESPOND)) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status zgodny z wpisem konfiguracji
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF,
						(GETINT(KONNF_OPCJE_IGNOREBYDEFAULT)) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF,
						(UIActionGetStatus( sUIAction( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF ) ) 
						& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny, czyli z powrotem w³aœciwy
				UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF,
						(UIActionGetStatus( sUIAction( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF ) ) 
						& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				}
				else//pacniêta kontrolka
				{
					ACTIONONLY( an );
					UIActionSetStatus ( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF,
						(UIActionGetStatus( sUIAction( KONNF_OPCJEID_GRUPA, KONNF_OPCJE_IGNOREIF ) ) 
						& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				}
			}
			break;
		//end skolima ADD
    }
    return 0;
}



int __stdcall IMessageProc(sIMessage_base * msgBase) {
    sIMessage_2params * msg = (msgBase->s_size>=sizeof(sIMessage_2params))?static_cast<sIMessage_2params*>(msgBase):0;
    switch (msgBase->id) {
    case IM_PLUG_NET: return konnfer::net;
    case IM_PLUG_TYPE: return IMT_CONTACT | IMT_MESSAGE | IMT_MSGUI | IMT_NETUID | IMT_NET;
    case IM_PLUG_VERSION: return 0;
    case IM_PLUG_SDKVERSION: return KONNEKT_SDK_V;
    case IM_PLUG_SIG: return (int)"KONNFER";
    case IM_PLUG_CORE_V: return (int)"W98";
    case IM_PLUG_UI_V: return 0;
    case IM_PLUG_NAME: return (int)"Konnferencja";
    case IM_PLUG_NETNAME: return (int)"Konferencja";
    case IM_PLUG_INIT: Plug_Init(msg->p1,msg->p2);return Init();
    case IM_PLUG_DEINIT: Plug_Deinit(msg->p1,msg->p2);return DeInit();
    case IM_PLUG_PRIORITY:     return PLUGP_LOW - 5;

    case IM_SETCOLS:         return ISetCols();

    case IM_UI_PREPARE: return IPrepare();
    case IM_START: return IStart();
    case IM_END: return IEnd();

    case IM_UIACTION: return ActionProc((sUIActionNotify_base*)msg->p1);

    // Obs³uga dodawania/usuwania kontaktów -----------------
    case IM_CNT_ADD:
        // Mo¿e jakaœ wtyczka albo skrypt chc¹ dodaæ kontakt?
        if (GETCNTI(msg->p1 , CNT_NET)==konnfer::net && std::find(groups.begin() , groups.end() , msg->p1) == groups.end())
            createGroupObject(msg->p1);
        break;
    case IM_CNT_REMOVE:
        if (GETCNTI(msg->p1 , CNT_NET)==konnfer::net)
            destroyGroupObject(msg->p1);
        break;
    case IM_CNT_CHANGED:{
        sIMessage_CntChanged * cc = static_cast<sIMessage_CntChanged*>(msgBase);
        if (!cc->_changed.net && !cc->_changed.uid) break;
        // Je¿eli zosta³ zmieniony kontakt, który by³ w konferencjach
        // najpierw trzeba usun¹æ jego obiekt
        if (cc->_oldNet==konnfer::net) {
            destroyGroupObject(cc->_cntID);
        }
        // Je¿eli aktualnie nadal jest obiektem konferencji... tworzymy go na nowo...
        if (GETCNTI(cc->_cntID , CNT_NET) == konnfer::net)
            createGroupObject(cc->_cntID);
        break;}

    // API gg.dll ---------------------------
    case IM_GG_EVENT:
        return handleGGEvent(static_cast<sIMessage_GGEvent*>(msgBase));
    //---------------------------------------------------
    case IM_MSG_RCV: {
        cMessage * m = (cMessage*) msg->p1;
        // Zamawiamy wysy³ane wiadomoœci konferencyjne do obs³ugi
        if (m->net == konnfer::net && (m->flag & MF_SEND))
            return IM_MSG_ok;
        return 0;}
    case IM_MSG_SEND: {
        cMessage * m = (cMessage*) msg->p1;
        if (m->net != konnfer::net) return 0;
        // Odszukujemy kontakt, do którego skierowana jest wiadomoœæ...
        int cnt = ICMessage(IMC_FINDCONTACT , m->net , (int)m->toUid);
        if (cnt <= 0) return 0;
        groups_it it = std::find(groups.begin() , groups.end() , cnt);
        // Wysy³amy tylko do grup, których obiekty posiadamy na stanie
        if (it == groups.end()) return 0;
        if ((*it)->sendMessage(m)) return IM_MSG_delete;// zosta³ wys³any - mo¿na ju¿ go usun¹æ
        return 0;}

    default:
        if (Ctrl) Ctrl->setError(IMERROR_NORESULT);
        return 0;

 }
 return 0;
}