/*
  KONNferencja - obs�uga konferencji dla protoko�u Gadu-Gadu

  (c)2003 Rafa� Lindemann / www.stamina.eu.org / www.konnekt.info

  Obsluga API Konnekta

  Kod udost�pniany na licencji GPL, kt�rej tre�� powinna by� dostarczona
  razem z tym kodem.
  Licensed under GPL.

  Modyfikacje:	Winthux	[11.07.2005]

*/

#include "stdafx.h"
#include <konnekt/konnferencja.h>
#include "konnferencja.h"
#include "skolimaUtilz.h"
// <code author="Winthux">
#include <konnekt/ui_message_controls.h>
#include <stamina/regex.h>

#pragma comment(lib, "pcre")
// </code>

using namespace konnfer;

namespace konnfer {
    gg_session * session = 0;
    std::deque<group_holder> groups;
	//<code author="winthux">
	int prevOwner;
	//</code>
};

int __stdcall DllMain(void * hinstDLL, unsigned long fdwReason, void * lpvReserved)
{
        return true;
}
int Init() {
    // Trzeba utworzy� obiekty dla istniej�cych kontakt�w
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
    /* Pr�bujemy zarejestrowa� si� w gg.dll */
    IMessage(IM_GG_REGISTERHANDLER,NET_GG,IMT_PROTOCOL,GGER_FIRSTLOOP|GGER_EVENT|GGER_LOGOUT);
    if (Ctrl->getError()==IMERROR_UNSUPPORTEDMSG) {// skoro nikt nie obs�u�y� tej wiadomo�ci, znaczy �e gg.dll jest wy��czone
        ICMessage(IMI_INFORM , (int)"Wtyczka Konnferencja nie b�dzie dzia�a� bez w��czonej wtyczki GaduGadu!");
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
	// definiujemy warto�� dla opcji
    SetColumn (DTCFG, Cfg::ignore_text, DT_CT_STR, "Konferencja {Display} nie zosta�a autoryzowana.Je�li chcesz rozmawia�, daj mi najpierw zna� prywatnie.", "Konnferencja/IgnoreText");
    SetColumn (DTCFG, Cfg::respond_to_whom, DT_CT_INT, 0, "Konnferencja/RespondToWhom");
    SetColumn (DTCFG, Cfg::respond, DT_CT_INT, 0, "Konnferencja/Respond");
    SetColumn (DTCFG, Cfg::ingore_by_default, DT_CT_INT, 1, "Konnferencja/IgrnoreByDefault");
	SetColumn (DTCNT, Cfg::kontakt_timestamp, DT_CT_INT, 0, "Konnferencja/LastMsgTimestamp" ); //niewidoczne dla usera
	SetColumn (DTCFG, Cfg::ignore_if, DT_CT_INT, 0, "Konnferencja/IgnoreIfUnknown");
	SetColumn (DTCFG, Cfg::show_template, DT_CT_STR, "{Display} [{UID}] {Status} {Info}", "Konnferencja/ShowUsersTemplate");
	// <code author="Winthux">
	SetColumn (DTCFG, Cfg::shift_tab, DT_CT_INT, 0, "Konnerencja/ShiftTab" );
	// </code>
    // ko�czymy dzia�anie funkcji
	//end skolima ADD
  return 1;
}

int IPrepare() {
	IconRegister(IML_16 , Ico::group_active , Ctrl->hDll() , 20000);
	IconRegister(IML_16 , Ico::group_inactive , Ctrl->hDll() , 20001);
	IconRegister(IML_16 , Ico::group_msg , Ctrl->hDll() , 20002);
	IconRegister(IML_16 , Ico::group_show , Ctrl->hDll() , 20003);
	//skolima ADD LINE
	IconRegister(IML_32 , Ico::ikona_32 , Ctrl->hDll() , 20004);
	IconRegister(IML_16 , UIIcon(IT_LOGO , konnfer::net , 0 , 0) , Ctrl->hDll() , 20000);
	UIActionInsert(IMIG_CNT , konnfer::Action::start_conference , 1 , ACTR_INIT , "Zacznij konferencj�" , Ico::group_active);
	UIActionInsert(IMIG_MSGTB , konnfer::Action::show_recipients , 1 , ACTR_INIT , "Wypisz rozm�wc�w" , Ico::group_show);
	UIActionInsert(IMIG_NFO_DETAILS , konnfer::Action::nfo_dummy , 0 , ACTS_HIDDEN | ACTR_INIT , "");

	//skolima ADD
	// dodajemy pozycje w okienku konfiguracyjnym
	UIGroupAdd (IMIG_GGCFG_USER, Cfg::id_grupa, 0, "KONNferencja", Ico::group_active);

	char bufor [1024] = "";
	wsprintf(bufor, "<br/>Copyright � 2004-05 <b>Stamina</b><br/>Autor i opiekun wtyczki: <b>Rafa� \"Hao\" Lindemann</b><br/>Modyfikacja: <b>Leszek \"Skolima\" Ciesielski</b><br/>Wykorzystano kod z wtyczki <b>K.Away</b> [<b>by Sija</b>] oraz <b>kPilot2</b> [<b>by Winthux</b>]<br/><br/>Wi�cej informacji i �r�d�a na stronie projektu http://kplugins.net/<br/>Data kompilacji: <b>%s</b> @ <b>%s</b>", __TIME__, __DATE__);
	UIActionCfgAddPluginInfoBox2(Cfg::id_grupa, "KONNferencja rozszerza mo�liwo�ci wtyczki Gadu-Gadu� - dodaje w niej obs�ug� rozm�w konferencyjnych. Konferencje pojawiaj� si� jako osobne kontakty, dzi�ki czemu mog� by� zapisane do p�niejszego u�ycia.", bufor, Icon32( Ico::ikona_32 ).c_str(),-4);

	if(ShowBits::checkLevel(ShowBits::levelNormal))
	{
		//nowa grupa
		UIActionAdd(Cfg::id_grupa , 0 , ACTT_GROUP , "Opcje ignorowania");
		//checkbox
		UIActionCfgAdd (Cfg::id_grupa, Cfg::ingore_by_default, ACTT_CHECK|ACTR_SHOW, 
			"Domy�lnie ignoruj nieznane konferencje :"
			, Cfg::ingore_by_default);
		//
		char res[250];
		//dropdown box 
		sprintf (res, " %s" CFGVALUE "1" "\n %s" CFGVALUE "0" "\n %s" CFGVALUE "2",
			"zawsze", "gdy nie znam kt�rego� rozm�wcy", "gdy nie znam �adnego z rozm�wc�w");
		UIActionCfgAdd (Cfg::id_grupa, Cfg::ignore_if,
			ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_NOICON ,
			res, Cfg::ignore_if, 0, 0, 260);
		//kolejny checkbox
		UIActionCfgAdd (Cfg::id_grupa, Cfg::respond, ACTT_CHECK, 
			"Odpisuj na wiadomo�ci od ignorowanych konferencji : "
			, Cfg::respond);
		// definiujemy pole combo 
		sprintf (res, " %s" CFGVALUE "1" "\n %s" CFGVALUE "0",
			"do ca�ej konferencji", "tylko do nadawcy");
		UIActionCfgAdd (Cfg::id_grupa, Cfg::respond_to_whom,
			ACTT_COMBO | ACTSCOMBO_LIST | ACTSCOMBO_NOICON ,
			res, Cfg::respond_to_whom, 0, 0, 260);
		//odpowied� dla ignorowanych
		UIActionAdd (Cfg::id_grupa, 0, ACTT_COMMENT, 
			"Na ignorowane wiadomo��i odpowiedz tekstem : "
			, 0, 0);
		UIActionCfgAdd ( Cfg::id_grupa, Cfg::ignore_text, ACTT_TEXT,
			"" CFGTIP "Rozpoznawane zmienne : \n{Display}", Cfg::ignore_text );
		//grup� trzeba zamkn��
		UIActionAdd(Cfg::id_grupa , 0 , ACTT_GROUPEND);

		if(ShowBits::checkLevel(ShowBits::levelAdvanced))
		{
			//nowa grupa
			UIActionAdd(Cfg::id_grupa , 0 , ACTT_GROUP , "Opcje okna wiadomo�ci");
			// <code author="Winthux">
			UIActionCfgAdd( Cfg::id_grupa, 0, ACTT_CHECK, "Dope�nianie nicka tylko samym klawiszem TAB",
				Cfg::shift_tab );
			// </code>
			//template show_users
			UIActionAdd (Cfg::id_grupa, 0, ACTT_COMMENT, 
				"Schemat wypisywania rozm�wc�w : "
				, 0, 0);
			UIActionCfgAdd ( Cfg::id_grupa, Cfg::show_template, ACTT_TEXT,
				"" CFGTIP "Rozpoznawane zmienne : \n{Display}\n{UID}\n{Status}\n{Info}", Cfg::show_template );
			//grup� trzeba zamkn�� Cfg::show_template
			UIActionAdd(Cfg::id_grupa , 0 , ACTT_GROUPEND);
		}
		else
			UIActionAdd (Cfg::id_grupa, 0, ACTT_COMMENT, 
			"Cz�� opcji jest niedost�pna ze wzgl�du na wybrany poziom u�ytkownika"
			, 0, 0);

	}
	else
		UIActionAdd (Cfg::id_grupa, 0, ACTT_COMMENT, 
		"Opcje s� niedost�pne ze wzgl�du na wybrany poziom u�ytkownika"
		, 0, 0);

	//end skolima ADD

	// <code author="Winthux">
	sUIActionInfo nfo( IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend);
	nfo.mask = UIAIM_ALL; // chcemy pobra� wszystko...
	nfo.txt = new char [100];
	nfo.txtSize = 99;
	UIActionGet(nfo); // pobieramy wszystkie dane
	// Zapisujemy w zmiennej globalnej (typu int) poprzedniego w�a�ciciela
	prevOwner = ICMessage(IMI_ACTION_GETOWNER , (int)&nfo.act);
	// Jak dostaniemy b��dny wynik (np. gdy jest stare UI) uznajemy �e w�a�cicielem jest interfejs
	if (prevOwner == 0) {prevOwner = ICMessage(IMC_PLUG_ID , 0);}
	// Niszczymy star� akcj�
	ICMessage(IMI_ACTION_REMOVE , (int)&nfo.act);
	// I reinkarnujemy (powstanie w tym samym miejscu, z tymi samymi parametrami! Nic nie trzeba ustawia�, chyba �e co� sami dodajemy...)
	ICMessage(IMI_ACTION , (int)&nfo);
	delete [] nfo.txt;
	// </code>

    return 1;
}

/*int ActionCfgProc(sUIActionNotify_base * anBase) {
  sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
  switch (anBase->act.id & ~IMIB_CFG) {
  }
  return 0;
}*/


// <code author="Winthux">
string Status( int st )
{
	switch(st)
	{
	case ST_ONLINE:
		return "Dost�pny";
	case ST_AWAY:
		return "Zaraz wracam";
	case ST_HIDDEN:
		return "Ukryty";
	case ST_OFFLINE:
		return "Niedost�pny";
	default:
		return "";
	}
	return "";
}

// funkcja zwraca ci�g znak�w za ostatni� spacj�, np. "Ala ma kota"
// w ret znajduje si� "kota" a w text "Ala ma "
string SplitString( string& text, int off, int& pos )
{
	int posL = (int)text.find_last_of( ' ', off-1 );
	int posR;
	string ret, v;

	if (posL == string::npos)	// drugi przypadek
	{
		posR = (int)text.find_first_of( ' ', off-1 );
		ret = text.substr( 0, posR );
		if (posR == string::npos)
			text.clear();
		else
            text = text.substr( posR );
		pos = 0;
	}
	else
	{
		posR = (int)text.find_first_of( ' ', off-1 );
		if (posR == string::npos)	// trzeci przypadek
		{
			ret = text.substr( posL+1 );
			text = text.substr( 0, posL+1 );
		}
		else	// pierwszy przypadek
		{
			v = text.substr( 0, posL+1 );
			ret = text.substr( posL+1, posR - posL - 1 );
			text = v + text.substr( posR );
		}
		pos = posL + 1;
	}
	return ret;
}

// przygotowuje nazwy kontaktow dla prega
// cnt1_display;cnt2_display;
string PrepareUIDs(int cnt)
{
	string uid = GETCNTC(cnt , CNT_UID);
	groupItems gi;
	std::stringstream body; body << ";";
	int net = getGroupUIDs(uid , gi);
	for (groupItems_it it = gi.begin(); it != gi.end(); it++) {
		int cnt_id = ICMessage(IMC_FINDCONTACT , net , (int)it->c_str());
		if (cnt != -1) 
		{
			body << GETCNTC(cnt_id , CNT_DISPLAY) << ";";
		}
	}
	return body.str();
}

WNDPROC msg_proc;
LRESULT CALLBACK msg_proc_new( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static unsigned int cTabPress = 0;
	static Stamina::RegEx *preg = NULL;
	static string entered;

	switch(uMsg)
	{
	case WM_CHAR:
		{
			SHORT key = LOWORD( VkKeyScan( (TCHAR)wParam ) );
			int shift = GETINT( Cfg::shift_tab );
			bool state = HIWORD( GetKeyState( VK_SHIFT ) )?true:false;
			// sprawdzamy czy shift zosta� naci�ni�ty razem z tabem
			// lub zosta�a zaznaczona opcja i sam tab zosta� nacisniety
			if( ( state && key == VK_TAB && !shift ) ||
				( !state && shift && key == VK_TAB ) )
			{
				// pobieramy id kontaktu, kt�rego jest to okno
				int cnt = (int)GetProp( hWnd, "CNT_ID" );
				// sprawdzamy dlugosc wpisanego tekstu, jezeli nic nie ma
				// to nic nie robimy :)
				int len = GetWindowTextLength( hWnd );
				if (len)
				{
                    char* szText = new char[len+1];
					static int off, pos;
					// pobieramy to co zosta�o wpisane
					GetWindowText( hWnd, szText, len+1 );
					string Text = szText;
					delete[] szText; szText = NULL;

					// dzielimy tekst
					if (entered.empty())
					{
						SendMessage( hWnd, EM_GETSEL, (WPARAM)&off, NULL );
                        entered = SplitString( Text, off, pos );
					}
					else	// je�eli ju� co� by�o, tzn, �e tab zosta� naci�ni�ty drugi raz
							// wi�c trzeba podzieli� w podobny spos�b z tym, �e usuwamy
							// co poprzednio wstawili�my ;)
						SplitString( Text, off, pos );

					string v = "/;("+ entered + ".*?);/i";
					if (!preg)
					{
						preg = new Stamina::RegEx;
						preg->setPattern( v );
						// przygotowujemy uidy
						preg->setSubject( PrepareUIDs( cnt ) );
					}

					if (preg->getStart())
						preg->setStart( preg->getStart() - 1 );

					// je�eli match zwraca jeden tzn. �e nic nie znalaz�
					// wi�c nic nie robimy
					if (preg->match_global() > 1)
					{
						Text.insert( pos, preg->getSub(1) );
						SetWindowText( hWnd, (LPCTSTR)Text.c_str() );
						int sel = pos+preg->getSub(1).length();
						SendMessage( hWnd, EM_SETSEL, (WPARAM)sel, (LPARAM)sel );
					}
					else if (!preg->isMatched())	// je�eli ju� nic nie znajduje
					{
						preg->reset();			// to resetujemy i zaczynamy od pocz�tku
						if (preg->match_global() > 1)
						{
							Text.insert( pos, preg->getSub(1) );
							SetWindowText( hWnd, (LPCTSTR)Text.c_str() );
							int sel = pos+preg->getSub(1).length();
							SendMessage( hWnd, EM_SETSEL, (WPARAM)sel, (LPARAM)sel );
						}
					}
				}
				return NULL;
			}
		}
		break;
	case WM_KEYDOWN:	// zosta� naci�ni�ty inny klawisz ni� tab wi�c czy�cimi pocz�tek
		{				// nicka i usuwamy prega
			int shift = GETINT( Cfg::shift_tab );
			bool state = HIWORD( GetKeyState( VK_SHIFT ) )?true:false;
			if ( !( ( state && wParam == VK_TAB && !shift ) ||
				( !state && shift && wParam == VK_TAB ) ) )
			{
				entered.clear();
				if (preg) { delete preg; preg = NULL; }
			}
		}
		break;
	}
	return CallWindowProc( msg_proc, hWnd, uMsg, wParam, lParam );
}
// </code>

ActionProc(sUIActionNotify_base * anBase) {
    sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
//    if ((anBase->act.id & IMIB_) == IMIB_CFG) return ActionCfgProc(anBase);
    switch (anBase->act.id) {
		/* <code author="Winthux"> */
		case Konnekt::UI::ACT::msg_ctrlsend:
			{
				int zwrot = IMessageDirect(IM_UIACTION , prevOwner , (int)anBase);
				// sprawdzamy czy okno nale�y do konferencji
				bool isConfer = GETCNTI(anBase->act.cnt, CNT_NET) == konnfer::net;
				if (isConfer)
				{
					if (anBase->code==ACTN_CREATEWINDOW)
					{     
						sUIActionNotify_createWindow * an = (anBase->s_size>=sizeof(sUIActionNotify_createWindow))?static_cast<sUIActionNotify_createWindow*>(anBase):0;
						if (an)	// subclassujemy okno
						{
                            msg_proc = (WNDPROC)SetWindowLongPtr( an->hwnd, GWLP_WNDPROC, (LONG_PTR)msg_proc_new);
							// ustawiamy id kontaktu, �eby wiedzie� do kogo nale�y okno
							SetProp( an->hwnd, "CNT_ID", (HANDLE)anBase->act.cnt );
						}
					}
				}
				return zwrot;
			}
			break;
			/* </code> */
        case Action::nfo_dummy:
            if (ACTN_CREATE) {
                // Pewne rzeczy wypada�oby ukry�
                bool isConfer = GETCNTI(anBase->act.cnt , CNT_NET) == konnfer::net;
                //UIActionSetStatus(sUIAction(IMIG_NFO_DETAILS , IMIB_CNT | CNT_NET) , isConfer?ACTS_HIDDEN:0 , ACTS_HIDDEN);
                // Lepiej, �eby u�ytkownik nie zmienia� UID'a...
                UIActionSetStatus(sUIAction(IMIG_NFO_DETAILS , IMIB_CNT | CNT_UID) , isConfer?ACTS_DISABLED:0 , ACTS_DISABLED);
            }
            return 0;
        case Action::show_recipients:
            if (anBase->code == ACTN_CREATE) {
                // Pewne rzeczy wypada�oby ukry�
                bool isConfer = GETCNTI(anBase->act.cnt , CNT_NET) == konnfer::net;
                UIActionSetStatus(anBase->act , isConfer?0:ACTS_HIDDEN , ACTS_HIDDEN);
            } else if (anBase->code == ACTN_ACTION) {
				//skolima ADD line
				string template_buff = GETSTR(Cfg::show_template,0,0);
                /*
                Informacj� wrzucamy jako wiadomo�� MT_QUICKEVENT
                do okna z konferencj�...
                */
                cMessage m;
                // wysy�amy do okna, w kt�rym zosta� naci�ni�ty
                // przycisk. Aby wys�a�, potrzebujemy UID, pobieramy wi�c go :)
                string uid = GETCNTC(anBase->act.cnt , CNT_UID);
                // Gdy nie wiemy kt�ry UID ustawi�, najlepiej ustawi� oba :)
                m.toUid = m.fromUid = (char*)uid.c_str();
                m.net = konnfer::net;
                m.type = MT_QUICKEVENT;
                std::stringstream body;// strumieniem wygodniej p�niej konwertowa� dane
                body << "W konferencji uczestnicz�:";
                groupItems gi;
                int net = getGroupUIDs(uid , gi);
                for (groupItems_it it = gi.begin(); it != gi.end(); it++) {
                    body << "\n";
                    int cnt = ICMessage(IMC_FINDCONTACT , net , (int)it->c_str());
                    if (cnt != -1) {
						//skolima ADD -> opis statusu
						string showSingleBuff = template_buff;
						string displayBuff;
						std::stringstream uidBuff;
						std::stringstream statusBuff;
						std::stringstream infoBuff;
						//zrobi� na zmiennych
                        displayBuff = GETCNTC(cnt , CNT_DISPLAY);
                        uidBuff << *it ;
						int cnt_stat = GETCNTI(cnt,CNT_STATUS);
						if(cnt_stat&ST_IGNORED)statusBuff << "Ignorowany";
						else
						{
							if(cnt_stat&ST_AWAY&ST_ONLINE)statusBuff << "Zaraz wraca";
							else if(cnt_stat&ST_ONLINE)statusBuff << "Dost�pny";
							if(cnt_stat==ST_OFFLINE)
							{
							    __time64_t ltime;
								_time64( &ltime );
								if((GETCNTI64(cnt,CNT_LASTACTIVITY))+20*60 >= ltime)
									statusBuff << "Niewidoczny";
								else
									statusBuff << "Niedost�pny";
							}
						}
						string sts_info = GETCNTC(cnt,CNT_STATUSINFO,0,0);
						if(!sts_info.empty())infoBuff << '\"' << sts_info << '\"';
						char charBuff[10000];
						//replaces
						sprintf(charBuff,"%s",displayBuff.c_str());
						showSingleBuff = StringReplace(showSingleBuff.c_str(),"{Display}",charBuff);
						sprintf(charBuff,"%s",uidBuff.str().c_str());
						showSingleBuff = StringReplace(showSingleBuff.c_str(),"{UID}",charBuff);
						sprintf(charBuff,"%s",statusBuff.str().c_str());
						showSingleBuff = StringReplace(showSingleBuff.c_str(),"{Status}",charBuff);
						sprintf(charBuff,"%s",infoBuff.str().c_str());
						showSingleBuff = StringReplace(showSingleBuff.c_str(),"{Info}",charBuff);
						//replaces done
						body << showSingleBuff;
						//end skolima ADD
                    }
					else body << '[' <<  *it << ']';
                }
                string body2 = body.str();
                m.body = (char*)body2.c_str();
                // Koniecznie trzeba ustawi�, aby to UI obs�u�y�o t� wiadomo��!
                m.flag = MF_HANDLEDBYUI;
                ICMessage(IMC_NEWMESSAGE , (int)&m);
                // UI obs�uguje QUICKEVENT ju� w IM_MSG_RCV. Nie ma wi�c potrzeby
                // wywo�ywania IMC_MESSAGE_QUEUE
            }
            return 0;
        case Action::start_conference: {
            switch (anBase->code) {
                case ACTN_CREATE:{
                    // Pewne rzeczy wypada�oby ukry�
                    bool isConfer = GETCNTI(anBase->act.cnt , CNT_NET) == konnfer::net;
                    UIActionSetStatus(sUIAction(IMIG_CNT , IMIA_CNT_IGNORE) , isConfer?ACTS_DISABLED:0 , ACTS_DISABLED);
                    // Sprawdzamy, czy s� warunki, aby wy�wietli� pozycj� w menu
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
                    /* Menu zosta�o wybrane... Zak�adamy, �e skoro zosta�o
                     naci�ni�te - by�o widoczne, a skoro by�o widoczne,
                     to wszystkie wymagane warunki zosta�y spe�nione...
                     i mo�emy tworzy� obiekt listy
                     Najpierw tworzymy kontakt o UIDzie z list� kontakt�w,
                     a Konnekt ju� sam wywo�a IM_CNT_ADD co automatycznie
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
                    // Tylko, gdy nie ma ju� tego samego...
                    groups_it gr_it = std::find(groups.begin() , groups.end() , groupContents_gg(selCount , uins));
                    if (gr_it == groups.end())
                        gr = new group_gg(selCount , uins , "" , true);
                    else
                        gr = static_cast<group_gg*>(**gr_it);
                    // Po utworzeniu kontaktu, otworzymy okno rozmowy...
                    // Udamy wi�c �e nacisn�li�my "Wy�lij wiadomo��"
                    UIActionCall(&sUIActionNotify_2params(sUIAction(IMIG_CNT , IMIA_CNT_MSGOPEN , gr->getCnt()) , ACTN_ACTION , 0 , 0));
                    return 0;}
            }
            break;}
	//skolima ADD
		case Cfg::respond :
			{
				ACTIONONLY( an );
				UIActionSetStatus ( Cfg::id_grupa, Cfg::respond_to_whom,
					(UIActionGetStatus( sUIAction( Cfg::id_grupa, Cfg::respond_to_whom ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				UIActionSetStatus ( Cfg::id_grupa, Cfg::ignore_text,
					(UIActionGetStatus( sUIAction( Cfg::id_grupa, Cfg::ignore_text ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
			}
			break;
		case Cfg::ingore_by_default :
			{
				if(anBase->code == ACTN_SHOW)//UI 'Ustawienia' b�dzie zaraz pokazywane
				{
					//dirty bugfix

				//ustawiam status zgodny z wpisem konfiguracji
				UIActionSetStatus ( Cfg::id_grupa, Cfg::respond_to_whom,
					(GETINT(Cfg::respond)) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny
				UIActionSetStatus ( Cfg::id_grupa, Cfg::respond_to_whom,
					(UIActionGetStatus( sUIAction( Cfg::id_grupa, Cfg::respond_to_whom ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny, czyli z powrotem w�a�ciwy
				UIActionSetStatus ( Cfg::id_grupa, Cfg::respond_to_whom,
					(UIActionGetStatus( sUIAction( Cfg::id_grupa, Cfg::respond_to_whom ) ) 
					& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status zgodny z wpisem konfiguracji
				UIActionSetStatus ( Cfg::id_grupa, Cfg::ignore_text,
					(GETINT(Cfg::respond)) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status zgodny z wpisem konfiguracji
				UIActionSetStatus ( Cfg::id_grupa, Cfg::ignore_if,
						(GETINT(Cfg::ingore_by_default)) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny
				UIActionSetStatus ( Cfg::id_grupa, Cfg::ignore_if,
						(UIActionGetStatus( sUIAction( Cfg::id_grupa, Cfg::ignore_if ) ) 
						& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				//ustawiam status odwrotny, czyli z powrotem w�a�ciwy
				UIActionSetStatus ( Cfg::id_grupa, Cfg::ignore_if,
						(UIActionGetStatus( sUIAction( Cfg::id_grupa, Cfg::ignore_if ) ) 
						& ACTS_DISABLED ) ? 0 : -1, ACTS_DISABLED );
				}
				else//pacni�ta kontrolka
				{
					ACTIONONLY( an );
					UIActionSetStatus ( Cfg::id_grupa, Cfg::ignore_if,
						(UIActionGetStatus( sUIAction( Cfg::id_grupa, Cfg::ignore_if ) ) 
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

    // Obs�uga dodawania/usuwania kontakt�w -----------------
    case IM_CNT_ADD:
        // Mo�e jaka� wtyczka albo skrypt chc� doda� kontakt?
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
        // Je�eli zosta� zmieniony kontakt, kt�ry by� w konferencjach
        // najpierw trzeba usun�� jego obiekt
        if (cc->_oldNet==konnfer::net) {
            destroyGroupObject(cc->_cntID);
        }
        // Je�eli aktualnie nadal jest obiektem konferencji... tworzymy go na nowo...
        if (GETCNTI(cc->_cntID , CNT_NET) == konnfer::net)
            createGroupObject(cc->_cntID);
        break;}

    // API gg.dll ---------------------------
    case IM_GG_EVENT:
        return handleGGEvent(static_cast<sIMessage_GGEvent*>(msgBase));
    //---------------------------------------------------
    case IM_MSG_RCV: {
        cMessage * m = (cMessage*) msg->p1;
        // Zamawiamy wysy�ane wiadomo�ci konferencyjne do obs�ugi
        if (m->net == konnfer::net && (m->flag & MF_SEND))
            return IM_MSG_ok;
        return 0;}
    case IM_MSG_SEND: {
        cMessage * m = (cMessage*) msg->p1;
        if (m->net != konnfer::net) return 0;
        // Odszukujemy kontakt, do kt�rego skierowana jest wiadomo��...
        int cnt = ICMessage(IMC_FINDCONTACT , m->net , (int)m->toUid);
        if (cnt <= 0) return 0;
        groups_it it = std::find(groups.begin() , groups.end() , cnt);
        // Wysy�amy tylko do grup, kt�rych obiekty posiadamy na stanie
        if (it == groups.end()) return 0;
        if ((*it)->sendMessage(m)) return IM_MSG_delete;// zosta� wys�any - mo�na ju� go usun��
        return 0;}

    default:
        if (Ctrl) Ctrl->setError(IMERROR_NORESULT);
        return 0;

 }
 return 0;
}