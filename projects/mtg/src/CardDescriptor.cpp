#include "../include/debug.h"
#include "../include/CardDescriptor.h"

CardDescriptor::CardDescriptor(): MTGCardInstance(){
	init();
}

int CardDescriptor::init(){
	return MTGCardInstance::init();
}

MTGCardInstance * CardDescriptor::match(MTGCardInstance * card){

	MTGCardInstance * match = card;
	for (int i = 0; i< nb_types; i++){

		if (!card->hasSubtype(types[i])){

			match = NULL;
		}
	}
	for (int i = 0; i< MTG_NB_COLORS; i++){
		if ((colors[i] == 1 && !card->hasColor(i))||(colors[i] == -1 && card->hasColor(i))){
			match = NULL;
		}
	}
	if ((tapped == -1 && card->isTapped()) || (tapped == 1 && !card->isTapped())){
		match = NULL;
	}
	return match;
}

MTGCardInstance * CardDescriptor::match(MTGGameZone * zone){
	return (nextmatch(zone, NULL));
}

MTGCardInstance * CardDescriptor::nextmatch(MTGGameZone * zone, MTGCardInstance * previous){
	int found = 0;
	if (NULL == previous) found = 1;
	for(int i=0; i < zone->nb_cards; i++){
		if(found && match(zone->cards[i])){
	#if defined (WIN32) || defined (LINUX)
char buf[4096];
sprintf(buf,"Card Descriptor MATCH!: %s \n" ,(zone->cards[i])->getName());
OutputDebugString(buf);
#endif
			return zone->cards[i];
		}
		if (zone->cards[i] == previous){
			found = 1;
		}
	}
	return NULL;
}
