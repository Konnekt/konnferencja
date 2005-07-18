/*
written for Imp-GG project
Copyright (C) 2005 Skolima (skolima.prv.pl)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// <code author="Winthux">
#include "stdafx.h"
// </code>
#include "skolimaUtilz.h"

char * cleanupUIDs(char* UIDlist,char* ownerUID)
{
	char * list = strtok(UIDlist,";");
	char * tokens[1000];
	char result[65000];
	int arr_pointer = 0;
	if(strcmp(list,ownerUID)!=0)tokens[arr_pointer++]=list;
	while((list = strtok(NULL, ";")) != NULL)//pociêcie
	{
		if(strcmp(list,ownerUID)!=0)tokens[arr_pointer++]=list;
	}
	for(int i=0;i<arr_pointer;i++)//bubble sort
	{
		bool changed = false;
		for(int j=0;j<arr_pointer-1;j++)
		{
			int compare = strcmp(tokens[j],tokens[j+1]);
			if(compare>0)
			{
				char * tmp = tokens[j];
				tokens[j] = tokens[j+1];
				tokens[j+1] = tmp;
				changed = true;
			}
		}
		if(!changed)break;//bubble sort end
	}
	for(int i=0;i<arr_pointer-1;i++)//delete duplicates
	{
		if((tokens[i]!=NULL&&tokens[i+1]!=NULL)&&(strcmp(tokens[i],tokens[i+1])==0))tokens[i+1]=NULL;
	}
	for(int i=0;i<arr_pointer;i++)//przepisanie
	{
		if(tokens[i]!=NULL)
		if(i==0)sprintf(result,"%s",tokens[i]);
		else sprintf(result,"%s;%s",result,tokens[i]);
	}
	sprintf(UIDlist,"%s",result);
	return UIDlist;
}

// zamiana tekstu w char by SIJA
// <code modifyBy="Winthux">
string StringReplace ( string text, const char * srch, char * chg )
{
	// szukamy pierwszego pokazania sie 'srch'
	size_t index = text.find ( srch );

	// pêtelka na dane
	while ( index != std::string::npos )
	{
		// zamieniamy
		text.replace ( index, strlen ( srch ), chg );
		// szukamy kolejnego
		index = text.find ( srch, index + strlen ( chg ) );
	}

	// zwracamy wynik
	return text;
}

string Icon32( int ico )
{
     char txt[32];
     std::string buff;
     
     sprintf( txt, "reg://IML32/%i.ICON", ico );     
     buff = AP_IMGURL;
     buff += txt;

     return buff;
}
// </code>