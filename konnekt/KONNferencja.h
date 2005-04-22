/* 
  KONNferencja - obs³uga konferencji dla protoko³u Gadu-Gadu

  (c)2003 Rafa³ Lindemann / www.stamina.eu.org / www.konnekt.info

  Nag³ówek do SDK

  Kod udostêpniany na licencji GPL, której treœæ powinna byæ dostarczona
  razem z tym kodem.
  Licensed under GPL.

*/ 

#pragma once

namespace konnfer {
    const int net = 20;

	namespace Action {
		const int start_conference = 20000;
		const int show_recipients = 20001;
		const int nfo_dummy = 20002;
	};
	// tabela kontaktów
	namespace Cnt {
	}
	namespace Ico {
		const int group_active = UIIcon(IT_STATUS , konnfer::net , ST_ONLINE , 0);
		const int group_inactive = UIIcon(IT_STATUS , konnfer::net , ST_OFFLINE , 0);
		const int group_msg = UIIcon(IT_MESSAGE , konnfer::net , MT_MESSAGE , 0);
		const int group_show = 20003;
	}

}