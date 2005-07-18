#pragma once

extern gg_session * session;

namespace konnfer {

    /*
    Dla przyspieszenia procesu obs³ugi, jak i umo¿liwienia
    w przysz³oœci doœæ ³atwego do³¹czenia innych sieci
    grupy trzymane s¹ we wtyczce jako lista obiektów. Niezale¿nie
    od ich reprezentacji w programie - czyli kontaktów na liœcie.
    */


    // lista kontaktów GG
    class groupContents_gg {
    protected:
        int count;
        uin_t * uins;
    public:
        groupContents_gg(int count=0,const uin_t * uins=0) {
            this->count = count;
            this->uins = 0;
            if (count) set(count , uins);
        }
        void set(int count , const uin_t* uins);
        void add(int count , const uin_t* uins);
        bool exists(uin_t uin);
        ~groupContents_gg();
        bool operator == (const groupContents_gg & other) const;
        int getCount() const {return count;}
        uin_t * getUins() const {return uins;}
    };


    // Klasa-Interfejs dla grup
    class group_base {
    protected:
        int cnt;// identyfikator kontaktu oznaczajacego grupe
        void createContact(std::string display , bool onList);
    public:
        group_base();
        virtual ~group_base();
        virtual bool sendMessage(cMessage * m)=0;// Wysyla wiadomosc do grupy
        //skolima REMOVE virtual void receiveMessage(cMessage * m , std::string from);// Wyœwietla wiadomoœæ od grupy uzupe³niaj¹c przy okazji pewne pola...
		void receiveMessage(cMessage * m , string from,string senderUID, int basicNet); //skolima ADD
        virtual int getNet()=0;// Zwraca sieæ kontaktów w grupie
        virtual bool operator==(const class groupContents_gg & test) const {return false;}// sprawdza, czy grupa zawiera podane kontakty z GG
        virtual std::string getUID()=0; //skolima -> wskazane, aby te UIDy by³y posortowane! (skolimaUtilz->cleanupUIDs sortuje leksykograficznie)
        bool operator==(const int cnt) const {return this->cnt == cnt;}
        int getCnt() {return cnt;}
        void setActive(bool active);
        // Tworzy kontakt na liœcie kontaktów
    };

    class group_gg : public group_base {
    public:
        groupContents_gg items;
        // Konstruktor, tworzy kontakt
        group_gg(int count , const uin_t * uins , std::string display , bool onList);
        // Konstruktor, nie tworzy kontaktu
        group_gg(int cnt, int count , const uin_t * uins);
        bool sendMessage(cMessage * m);
        int getNet() {return NET_GG;}
        bool operator==(const groupContents_gg & test) const ;
        std::string getUID();
    };
    // Klasa do trzymania obiektów w deque
    class group_holder {
    private:
        group_base * p;
    public:
        group_holder(group_base * p = 0) {this->p = p;}
        group_base * operator * () {return p;}
        group_base * operator -> () {return p;}
        bool operator==(const int cnt) const {return *p == cnt;}
        bool operator==(const group_base * p) const {return this->p == p;}
        bool operator==(const group_base & p) const {return this->p == &p;}
        bool operator==(const class groupContents_gg & test) const {return *p == test;}
    };

    // lista grup
    extern std::deque<group_holder> groups;
    typedef std::deque<group_holder>::iterator groups_it;
    // uzywane do rozbijania UIDa konferencji, na UIDy kontaktów
    typedef std::deque<string> groupItems;
    typedef std::deque<string>::iterator groupItems_it;


    // Tworzy / niszczy obiekty w groups dla kontaktów na liœcie
    void createGroupObject(int cnt);
    void destroyGroupObject(int cnt);
    int handleGGEvent(sIMessage_GGEvent * e);
    int getGroupUIDs(string uid , groupItems & items);


	//<code author="Skolima" modifyBy="Winthux"> 
	namespace Cfg
	{
		const int ignore_text = net*1000+10;
		const int respond_to_whom = net*1000+11;
		const int respond = net*1000+12;
		const int ingore_by_default = net*1000+13;
		const int id_grupa = net*1000+14;
		const int kontakt_timestamp = net*1000+15;
		const int ignore_if = net*1000+16;
		const int show_template = net*1000+18;
		const int shift_tab = net*1000+19;
	};

	namespace Ico
	{
		const int ikona_32 = net*1000+17;	// zostawi³em, ¿eby by³o kompatybilne w dó³
		// kolejne wartosci ikon najlepiej zaczyna od net*1000+100
	};
//#define KONNF_OPCJE_IGNORETEXT net*1000+10
//#define Cfg::respond_to_whom net*1000+11
//#define KONNF_OPCJE_RESPOND net*1000+12
//#define KONNF_OPCJE_IGNOREBYDEFAULT net*1000+13
//#define KONNF_OPCJEID_GRUPA net*1000+14
//#define KONNF_OPCJE_KONTAKT_TIMESTAMP net*1000+15
//#define KONNF_OPCJE_IGNOREIF net*1000+16
//#define KONNF_IKONA_32 net*1000+17
//#define KONNF_OPCJE_SHOWTEMPLATE net*1000+18

//</code>

}

using namespace konnfer;