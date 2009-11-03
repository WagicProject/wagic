#include "../include/config.h"
#include "../include/AIStats.h"
#include "../include/GameObserver.h"
#include "../include/Player.h"
#include "../include/MTGCardInstance.h"
#include "../include/WEvent.h"

bool compare_aistats(AIStat * first, AIStat * second){
  float damage1 = first->value / first->occurences;
  float damage2 = second->value/ second->occurences;
  return (damage1 > damage2);
}

AIStats::AIStats(Player * _player, char * _filename){
  filename = _filename;
  load(_filename);
  player = _player;
}

AIStats::~AIStats(){
  list<AIStat *>::iterator it;
  for ( it=stats.begin() ; it != stats.end(); it++ ){
    AIStat * stat = *it;
    delete stat;
  }
}

void AIStats::updateStatsCard(MTGCardInstance * cardInstance, Damage * damage, float multiplier){
  MTGCard * card = cardInstance->model;
  AIStat * stat = find(card);
  if (!stat){
    stat = NEW AIStat(card->getMTGId(),0,1,0);
    stats.push_back(stat);
  }
  if (damage->target == player){
    stat->value+= multiplier * STATS_PLAYER_MULTIPLIER * damage->damage;
  }else if (damage->target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE){
    MTGCardInstance * target = (MTGCardInstance *)damage->target;
    if (target->controller() == player && !target->isInPlay()){
      //One of my creatures got lethal damage...
      stat->value+= multiplier * STATS_CREATURE_MULTIPLIER * damage->damage;
    }
  }
}

int AIStats::receiveEvent(WEvent * event){
  WEventDamage * e = dynamic_cast<WEventDamage *>(event);
  if (!e) return 0; //we take only Damage events into accountright now
  Damage * damage = e->damage;
  MTGGameZone * opponentZone = player->opponent()->game->inPlay;

  MTGCardInstance * card = damage->source;
  updateStatsCard(card,damage);

  //Auras on damage source can be the cause
  for (int i = 0;  i < opponentZone->nb_cards; ++i){
    MTGCardInstance * aura = opponentZone->cards[i];
    if (aura->target == card){
      updateStatsCard(aura,damage, STATS_AURA_MULTIPLIER);
    }
  }
  stats.sort(compare_aistats); //this could be slow, if it is, let's run it only at the end of the turn
  return 1; //is this meant to return 0 or 1?
}

bool AIStats::isInTop(MTGCardInstance * card, unsigned int max, bool tooSmallCountsForTrue ){
  if (stats.size()<max) return tooSmallCountsForTrue;
  unsigned int n = 0;
  MTGCard * source = card->model;
  int id = source->getMTGId();
  list<AIStat *>::iterator it;
  for ( it=stats.begin() ; it != stats.end(); it++ ){
    if (n >= max) return false;
    AIStat * stat = *it;
    if (stat->source == id){
      if (stat->value>=0) return true;
      return false;
    }
    n++;
  }
  return false;
}

AIStat * AIStats::find(MTGCard * source){
  int id = source->getMTGId();
  list<AIStat *>::iterator it;
  for ( it=stats.begin() ; it != stats.end(); it++ ){
    AIStat * stat = *it;
    if (stat->source == id) return stat;
  }
  return NULL;
}

void AIStats::load(char * filename){
  std::ifstream file(filename);
  std::string s;

  if(file){
    while(std::getline(file,s)){
      int cardid = atoi(s.c_str());
      std::getline(file,s);
      int value = atoi(s.c_str());
      std::getline(file,s);
      int direct = atoi(s.c_str());
      AIStat * stat = NEW AIStat(cardid,value,1,direct);
      stats.push_back(stat);
    }
    file.close();
  }else{
    //TODO Error management
  }
}
void AIStats::save(){
  std::ofstream file(filename.c_str());
  char writer[128];
  if (file){
    list<AIStat *>::iterator it;
    for ( it=stats.begin() ; it != stats.end(); it++ ){
      AIStat * stat = *it;
      if (stat->value > 0){
	sprintf(writer,"%i\n%i\n%i\n", stat->source,stat->value/2,stat->direct);
	file<<writer;
      }
    }
    file.close();
  }

}
