/*
  KONNferencja - obs³uga konferencji dla protoko³u Gadu-Gadu

  (c)2003 Rafa³ Lindemann / www.stamina.eu.org / www.konnekt.info

  Obsluga GaduGadu

  Kod udostêpniany na licencji GPL, której treœæ powinna byæ dostarczona
  razem z tym kodem.
  Licensed under GPL.

  zmodyfikowane przez skolimê

*/

#include "stdafx.h"
#include <konnekt/konnferencja.h>
#include "konnferencja_main.h"
//skolima ADD
#include "skolimaUtilz.h"
//end skolima ADD

// groupContents_gg --------------------------------------------------
void groupContents_gg::set(int count , const uin_t* uins) {
    if (this->uins) delete [] this->uins;
    this->uins = new uin_t[count];
    this->count = count;
    for (int i=0; i<count; i++)
        this->uins[i] = uins[i];
}
bool groupContents_gg::exists(uin_t uin) {
    if (!this->uins) return false;
    for (int i=0; i<this->count; i++)
        if (this->uins[i] == uin) return true;
    return false;
}

void groupContents_gg::add(int count , const uin_t* uins) {
    if (!this->uins) {set(count , uins); return;}
    uin_t * temp = new uin_t[this->count + count];
    // Kopiujemy to co by³o
    for (int i=0; i<this->count; i++)
        temp[i] = this->uins[i];
    delete this->uins;
    this->uins = temp;
    // Dodajemy nowe, ale tylko jeœli siê nie powtarzaj¹
    for (int i=0; i<count; i++)
        if (!exists(uins[i]))
            this->uins[this->count++] = uins[i];
}

groupContents_gg::~groupContents_gg() {
    if (this->uins) delete [] this->uins;
}
// Zwykle porownanie, tyle ze niezaleznie od kolejnosci wystepowania kontaktow
bool groupContents_gg::operator ==(const groupContents_gg & other) const {
    if (this->count != other.count) return false;
    for (int i=0; i<this->count; i++) {
        int j;
        for (j=0; j<this->count; j++) {
            // Jezeli znajdzie gdzies w srodku, sprawdza nastêpny
            if (this->uins[i] == other.uins[j]) break;
        }
        // Skoro przelecia³ wszystkie i nie znalaz³, znaczy ¿e nie s¹ te same
        if (j >= this->count) return false;
    }
    return true;
}


// group_gg

group_gg::group_gg(int count , const uin_t * uins , string display , bool onList) {
    items.set(count , uins);
    createContact(display , onList);
    if (IMessage(IM_ISCONNECTED , NET_GG , IMT_PROTOCOL))
        setActive(true);
}
group_gg::group_gg(int cnt, int count , const uin_t * uins) {
    items.set(count , uins);
    this->cnt = cnt;
}
bool group_gg::sendMessage(cMessage * m) {
    // Pobieramy obiekt sesji z GG
    gg_session * sess = (gg_session*) IMessage(IM_GG_GETSESSION , NET_GG , IMT_PROTOCOL);
    if (sess) {
        int result = gg_send_message_confer(sess , GG_CLASS_CHAT , items.getCount() , (uin_t*)items.getUins() , (const unsigned char*)m->body);
        // Zwalniamy obiekt sesji
        IMessage(IM_GG_RELEASESESSION , NET_GG , IMT_PROTOCOL);
        return result!=-1;
    } else return false;
}
bool group_gg::operator==(const groupContents_gg & test) const {
    return test == items;
}
string group_gg::getUID() {
    if (cnt>0) return GETCNTC(cnt , CNT_UID);
    // Skoro jeszcze go nie generowaliœmy, trzeba to zrobiæ po raz pierwszy...
    std::stringstream uid;
    for (int i=0; i<items.getCount(); i++) {
        if (i) uid << ";";
        uid << items.getUins()[i];
    }
    //skolima REMOVE uid << "@10";
	//skolima ADD sortowanie uidów leksykograficzne
	char KonnektOwnerUID[10],buff[65000];
	sprintf(KonnektOwnerUID,"%d",GETINT(1053));//nasz uid na gadu
	sprintf(buff,"%s",uid.str().c_str());//lista uidów jako char*
	sprintf(buff,"%s@%d",cleanupUIDs(buff,KonnektOwnerUID),NET_GG);//hard-coded GG_NET !!
	std::string uid_str = buff;
	//end skolima ADD
    return uid_str;
}


// ggEvents --------------------------------------------------
int konnfer::handleGGEvent(sIMessage_GGEvent * event) {
    switch (event->eventType) {
        case GGER_FIRSTLOOP: case GGER_LOGOUT: {
            // Ustawiamy statusy konferencji na w³¹czone i wy³¹czone...
            bool active = event->eventType == GGER_FIRSTLOOP;
            if (active) {
                // Skoro siê po³¹czyliœmy, wypada³oby przejrzeæ kolejkê, czy nie ma czegoœ do wys³ania...
                ICMessage(IMC_MESSAGEQUEUE , (int)&sMESSAGESELECT(konnfer::net , 0 , MT_MESSAGE , MF_SEND));
            }
            for (groups_it it=groups.begin(); it!=groups.end(); it++)
                if ((*it)->getNet()==NET_GG)
                    (*it)->setActive(active);
            break;}
        case GGER_EVENT: {
            gg_event * e = event->data.event;
            switch (e->type) {
                case GG_EVENT_MSG: {
                    if (e->event.msg.msgclass == GG_CLASS_CTCP
                        || !e->event.msg.message
                        || !*e->event.msg.message || e->event.msg.recipients_count==0) break;
                    /* Sprawdzamy, czy mamy grupê na liœcie...
                     Jeœli nie, tworzymy j¹... Pomijamy w ten sposób
                     obs³ugê wiadomoœci spoza listy w interfejsie,
                     która w naszym przypadku, mog³aby nie zadzia³aæ
                     najlepiej */
                    // Tworzymy kopiê odbiorców i dorzucamy wysy³aj¹cego do listy
                    groupContents_gg gc(e->event.msg.recipients_count , e->event.msg.recipients);
                    gc.add(1 , &e->event.msg.sender);
                    groups_it it = std::find(groups.begin() , groups.end() , gc);
                    group_gg * gr;
                    if (it != groups.end())
                        gr = static_cast<group_gg*>(**it);
                    else {
                        gr = new group_gg(gc.getCount() , gc.getUins() , "" , false);
                    }
                    cMessage m;
                    m.body = (char*)e->event.msg.message;
                    // Ustawiamy czas nadejœcia wiadomoœci...
                    // Je¿eli okno rozmowy jest zamkniête, ustawiamy
                    // na czas ustawiony w wiadomoœci, chyba ¿e
                    // na komputerze jest wczeœniejsza godzina...

					/*skolima changed - old was
					m.time = ICMessage(IMI_MSG_WINDOWSTATE , gr->getCnt())==0?
                        min(_time64(0) , e->event.msg.time)
                        :_time64(0)
                        ;
					*/
                    m.time = min(_time64(0) , e->event.msg.time);
                    char buff [32];
                    itoa(e->event.msg.sender , buff , 10);
                    string from = GETCNTC( ICMessage(IMC_FINDCONTACT , NET_GG , (int)buff) , CNT_DISPLAY);
                    if (from.empty()) from = buff;
                    gr->receiveMessage(&m , from, buff, NET_GG);//skolima added buff, NET_GG
                    return GGERF_ABORT;}
            }
            break;}
    }
    return 0;
}