/*
  KONNferencja - obs�uga konferencji dla protoko�u Gadu-Gadu

  (c)2003 Rafa� Lindemann / www.stamina.eu.org / www.konnekt.info

  Klasa group_base

  Kod udost�pniany na licencji GPL, kt�rej tre�� powinna by� dostarczona
  razem z tym kodem.
  Licensed under GPL.

  zmodyfikowane przez skolim�

*/

#include "stdafx.h"
//skolima ADD
#include "skolimaUtilz.h"
//end skolima ADD
#include <konnekt/KONNferencja.h>
#include <konnekt/kAway.h>
#include "konnferencja_main.h"


void konnfer::createGroupObject(int cnt) {
    // Sprawdzamy czy kontakt jest "nasz"
    if (GETCNTI(cnt , CNT_NET)!= konnfer::net) return;
    std::string uid = GETCNTC(cnt , CNT_UID);
    int net;
    size_t pos = uid.find_last_of('@');
    if (pos == -1) {
        net = NET_GG;
    } else {
        net = atoi(uid.substr(pos+1).c_str());
        uid.erase(pos);
    }
    if (net == NET_GG) {
        uin_t uins [100];
        int count = 0;
		//skolima ADD sortowanie uid�w leksykograficzne
		char KonnektOwnerUID[10],buff[65000];
		sprintf(KonnektOwnerUID,"%d",GETINT(1053));//nasz uid na gadu
		sprintf(buff,"%s",uid.c_str());//lista uid�w jako char*
		uid = cleanupUIDs(buff,KonnektOwnerUID);
		sprintf(buff,"%s@%d",uid.c_str(),net);
		SETCNTC(cnt , CNT_UID,buff);
		ICMessage(IMC_CNT_CHANGED , cnt);
		//end skolima ADD
        while (uid.size()) {
            // atoi sam wczyta numerek tylko do �rednika...
            uins[count++] = atoi(uid.c_str());
            pos = uid.find(';');
            if (pos == -1) uid.clear();
            else uid.erase(0 , pos+1);
        }
        new group_gg(cnt , count , uins);
    }
}
void konnfer::destroyGroupObject(int cnt) {
    groups_it it = std::find(groups.begin() , groups.end() , cnt);
    if (it != groups.end()) {
        delete **it;
    }
}

int konnfer::getGroupUIDs(string uid , groupItems & items) {
    size_t pos;
    pos = uid.find_last_of('@');
    int net = NET_GG;
    if (pos!=-1) {
        net = atoi(uid.substr(pos+1).c_str());
        uid.erase(pos);
    }
    items.clear();
    while (uid.size()) {
        // atoi sam wczyta numerek tylko do �rednika...
        pos = uid.find(';');
        items.push_back(uid.substr(0,pos));
        if (pos == -1) uid.clear();
        else uid.erase(0 , pos+1);
    }
    return net;
}



// group_base ----------------------------------------------------
group_base::group_base() {
    // wstawia si� na list� grup
    cnt = -1;
    groups.push_back(group_holder(this));
    IMLOG("group_base() size=%d" , groups.size());
}
group_base::~group_base() {
    // usuwa sam siebie z listy grup
    _ASSERTE(_CrtCheckMemory());
    groups_it it = std::find(groups.begin(), groups.end() , this);
    if (it!=groups.end()) groups.erase(it);
    _ASSERTE(_CrtCheckMemory());
}
void group_base::receiveMessage(cMessage * m , string from,string senderUID, int basicNet) {
    // w m juz siedza prawie wszystkie informacje, teraz trzeba je tylko
    // troche uzupelnic i wrzucic do kolejki...
    m->flag |= MF_HANDLEDBYUI;// Wiadomo�� ma zosta� w pe�ni obs�u�ona przez interfejs
    m->net = konnfer::net;// Ustawiamy sie� naszej wtyczki...
    string uid = this->getUID();
    m->fromUid = (char*)uid.c_str();// Ustawiamy te� odpowiedni UID
    m->toUid = "";
    m->type = MT_MESSAGE;// Ca�o�� wysy�ana jest jako zwyk�a wiadomo��
    m->notify = Ico::group_msg;
    // Jako �e wiadomo�� obs�ugiwana jest przez interfejs, wysy�ana
    // jest jako cz�� zwyk�ej rozmowy. UI w wersji 0.1.18 przyjmuje
    // w parametrach dodatkowych MEX_DISPLAY i u�ywa tego jako
    // autora wypowiedzi...
    string ext = SetExtParam(m->ext , MEX_DISPLAY , from);
    m->ext = (char*)ext.c_str();

	//skolima ADD - obs�uga ignorowania
	int cnt = ICMessage(IMC_FINDCONTACT , konnfer::net , (int)this->getUID().c_str());
	int cnt_stat = GETCNTI(cnt, CNT_STATUS);
	if (cnt_stat&ST_IGNORED)
	{
		//wypada�oby doda� j� do histori w odpowiednim miejscu
		sHISTORYADD olany;
		olany.m = m;
		olany.dir = "deleted";
		olany.name = "ignorowani";
		ICMessage(IMI_HISTORY_ADD , (int)&olany);//dodaje do historii
		if(GETINT (KONNF_OPCJE_RESPOND))
		{
			// ustawiamy aktualny czas
            __time64_t ltime;
            _time64( &ltime );
			if ( GETCNTI64 ( cnt, KONNF_OPCJE_KONTAKT_TIMESTAMP ) + 30 >= ltime )return;//za wczesnie na odpowied� - antiflood
			SETCNTI64 ( cnt, KONNF_OPCJE_KONTAKT_TIMESTAMP, ltime );
			string bodyBuff = GETSTR(KONNF_OPCJE_IGNORETEXT,0,0);
			char charBuff[10000];
			sprintf(charBuff,"%s",GETCNTC( ICMessage(IMC_FINDCONTACT , m->net , (int)m->fromUid) , CNT_DISPLAY));
			//replaces
			bodyBuff = StringReplace(bodyBuff.c_str(),"{Display}",charBuff);
			//replaces done
			char bodyBuffchar[10000];
			sprintf(bodyBuffchar,"%s",bodyBuff.c_str());
			cMessage msg;
			msg.flag = MF_SEND;//msg.flag = MF_SEND|MF_HTML;
			
			if(GETINT (KONNF_OPCJE_RESPONDTOWHOM)==0)
			{//tylko do nadawcy
				char buff[32];				
				sprintf(buff,"%s",senderUID.c_str());
				msg.toUid = buff;
				msg.net = basicNet;
			}
			else if(GETINT (KONNF_OPCJE_RESPONDTOWHOM)==1)
			{//do ca�ej konnferencji
				msg.toUid = m->fromUid;
				msg.net = m->net;
			}			

			msg.time = m->time;
			msg.type = m->type;
			msg.body = bodyBuffchar;
			sMESSAGESELECT msc;            
			msc.id = ICMessage ( IMC_NEWMESSAGE , (int) &msg );            
			if (msc.id) ICMessage ( IMC_MESSAGEQUEUE , (int) &msc );
		}
		return;
		//ko�czymy, bo ignorujemy ten kontakt
	}
	//end skolima ADD

	sMESSAGESELECT ms;
    ms.id = ICMessage(IMC_NEWMESSAGE , (int)m);

    if (ms.id && ms.id != -1)
        ICMessage(IMC_MESSAGEQUEUE , (int)&ms);
}
void group_base::createContact(string display , bool onList) {

    this->cnt = ICMessage(IMC_CNT_ADD , konnfer::net , (int)this->getUID().c_str());
	bool isUnknown = false;//skolima ADD <- this line
    if (display.empty()) {
        // Trzeba wykombinowa� jaki� display...
        groupItems gi;
        int net = getGroupUIDs(this->getUID() , gi);
        for (groupItems_it it = gi.begin(); it != gi.end(); it++) {
            if (!display.empty()) display+=", ";
            int cnt = ICMessage(IMC_FINDCONTACT , net , (int)it->c_str());
			//skolima OLD was if (cnt != -1)
            if (cnt != -1&&strlen(GETCNTC(cnt , CNT_DISPLAY))!=0)
                display += GETCNTC(cnt , CNT_DISPLAY);
            else
			{
				display+=*it;
				//skolima ADD
				if(cnt==-1)isUnknown = true;
				//end skolima ADD
			}
        }
    }
    SETCNTC(this->cnt , CNT_DISPLAY , display.c_str());
	SETCNTI(this->cnt , CNT_STATUS , ST_OFFLINE | (onList?0:ST_NOTINLIST));
	//skolima ADD
	//chcemy, �eby konnferencja NIE dostawa�a domy�lnie kAwaya
	SETCNTI(this->cnt , kID_OPT_CNT_NOSILENTAWAY , 2);
	SETCNTI(this->cnt , kID_OPT_CNT_NOSILENTON , 2);
	SETCNTI(this->cnt , kID_OPT_CNT_NOSILENTOFF , 2);
	//je�li user tak ustawi�, domyslnie wrzucamy nowe kontakty jako ignorowane...
	if(!onList&&GETINT (KONNF_OPCJE_IGNOREBYDEFAULT)==1 &&
		(GETINT (KONNF_OPCJE_IGNOREIF)==1||(GETINT (KONNF_OPCJE_IGNOREIF)==0&&isUnknown)))
	{
		//ignorujemy....
		ICMessage(IMC_IGN_ADD, konnfer::net, (int)this->getUID().c_str());
	}
	//end skolima ADD
    ICMessage(IMC_CNT_CHANGED , this->cnt);
}
void group_base::setActive(bool active) {
    sIMessage_StatusChange sc(IMC_CNT_SETSTATUS , this->cnt , active?ST_ONLINE:ST_OFFLINE , 0);
    Ctrl->IMessage(&sc);
    ICMessage(IMI_REFRESH_CNT , this->cnt);
}