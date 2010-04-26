#include "../include/StoryFlow.h"
#include "../include/MTGDefinitions.h"
#include "../include/config.h"
#include "../include/WResourceManager.h"
#include "../include/AIPlayer.h"
#include "../include/Rules.h"
#include <JLBFont.h>
#include <JGE.h>
#include <JFileSystem.h>


StoryGraphicalElement::StoryGraphicalElement(float x, float y): JGuiObject(0), mX(x),mY(y) {
}

StoryText::StoryText(string text, float _mX, float _mY, string _align):StoryGraphicalElement(_mX,_mY), text(text) {
  align = JGETEXT_LEFT;
  if (_align.compare("center") == 0) {
    align = JGETEXT_CENTER;
  }else if (_align.compare("right") == 0) {
    align = JGETEXT_RIGHT;
  }
  if (align == JGETEXT_CENTER && mX == 0){
    mX = SCREEN_WIDTH/2;
  }
}
void StoryText::Render() {
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);
  mFont->SetColor(ARGB(200,255,255,255));
  mFont->SetScale(1.0);
  mFont->DrawString(text.c_str(), mX, mY, align);
}
  void StoryText::Update(float dt){
    //Nothing for now
  }

  ostream& StoryText::toString(ostream& out) const
{
  return out << "StoryText ::: text : " << text;
}

  StoryImage::StoryImage(string img, float mX, float mY):StoryGraphicalElement(mX,mY), img(img) {

}
void StoryImage::Render() {
  JQuad * quad = resources.RetrieveQuad(img);
  if (quad) {
    float x = mX;
    if (mX == -1) {
      x = SCREEN_WIDTH/2;
      quad->SetHotSpot(quad->mWidth/2, 0);
    }
    JRenderer::GetInstance()->RenderQuad(quad,x, mY);
  }
}
  void StoryImage::Update(float dt){
    //Nothing for now
  }

  ostream& StoryImage::toString(ostream& out) const
{
  return out << "StoryImage ::: img : " << img;
}

StoryPage::StoryPage(StoryFlow * mParent):mParent(mParent){
}



StoryFlow::StoryFlow(string folder): folder(folder){
  string path = "campaigns/";
  path.append(folder).append("/story.xml");
  parse(path);
}



void  StoryChoice::Render()
{
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);
  mFont->SetColor(ARGB(200,255,255,255));
  if (mHasFocus) mFont->SetColor(ARGB(255,255,255,0));
  mFont->SetScale(mScale);
  mFont->DrawString(text.c_str(), mX, mY, JGETEXT_CENTER);
}

void  StoryChoice::Update(float dt)
{
    if (mScale < mTargetScale)
    {
      mScale += 8.0f*dt;
      if (mScale > mTargetScale)
	mScale = mTargetScale;
    }
  else if (mScale > mTargetScale)
    {
      mScale -= 8.0f*dt;
      if (mScale < mTargetScale)
	mScale = mTargetScale;
	}
}


void StoryChoice::Entering()
{
  mHasFocus = true;
  mTargetScale = 1.2f;
}


bool  StoryChoice::Leaving(JButton key)
{
  mHasFocus = false;
  mTargetScale = 1.0f;
  return true;
}


bool StoryChoice::ButtonPressed()
{
  return true;
}



bool StoryChoice::hasFocus()
{
  return mHasFocus;
}

ostream& StoryChoice::toString(ostream& out) const
{
  return out << "StoryChoice ::: mHasFocus : " << mHasFocus;
}


StoryChoice::StoryChoice(string pageId, string text, int JGOid, float mX, float mY, bool hasFocus):JGuiObject(JGOid),pageId(pageId),text(text),mX(mX),mY(mY),mHasFocus(hasFocus){
    mScale = 1.0f;
  mTargetScale = 1.0f;
  if(hasFocus) mTargetScale = 1.2f;
}

//Actually loads "game"
void StoryDuel::init(){
  Player * players[2];

  char folder[255], deckFile[255],deckFileSmall[255];
  sprintf(folder, CAMPAIGNS_FOLDER"%s/%s" ,mParent->folder.c_str(), pageId.c_str());

  sprintf(deckFile, "%s/deck.txt", folder);
  MTGDeck * tempDeck = NEW MTGDeck(deckFile, GameApp::collection);
  sprintf(deckFileSmall, "campaign_%s",  mParent->folder.c_str());
  players[0] = NEW HumanPlayer(NEW MTGPlayerCards(tempDeck),deckFile,deckFileSmall);
  SAFE_DELETE(tempDeck);

  sprintf(deckFile,"%s/ennemy_deck.txt", folder);
  tempDeck = NEW MTGDeck(deckFile, GameApp::collection);
  sprintf(deckFileSmall, "campaign_ennemy_%s_%s",  mParent->folder.c_str(), pageId.c_str());
  players[1] = NEW AIPlayerBaka(NEW MTGPlayerCards(tempDeck),deckFile,deckFileSmall,"baka.jpg");
  SAFE_DELETE(tempDeck);

  string rulesFile = folder;
  rulesFile.append("/rules.txt");
  rules = NEW Rules(rulesFile);

  GameObserver::Init(players, 2);
  game = GameObserver::GetInstance();
	game->startGame(rules);

}
StoryDuel::StoryDuel(TiXmlElement* root,StoryFlow * mParent): StoryPage(mParent) {
  game = NULL;
  rules = NULL;
  pageId = root->Attribute("id");
  for (TiXmlNode* node = root->FirstChild(); node; node = node->NextSibling()) {
	  TiXmlElement* element = node->ToElement();
	  if (element) {
		  if (strcmp(element->Value(), "onwin")==0) {
          const char* textC = element->GetText();
          onWin = textC;
		  }
      else if (strcmp(element->Value(), "onlose")==0) {
          const char* textC = element->GetText();
          onLose = textC;
		  }else {
        //Error
      }
	  }
	}
}

StoryDuel::~StoryDuel(){
  SAFE_DELETE(rules);
  if(game)GameObserver::EndInstance();
  game=NULL;
}

void StoryDuel::Update(float dt){
  if (!game) init();
  game->Update(dt);
  if (game->gameOver){
    if (game->gameOver == game->players[1]) mParent->gotoPage(onWin);
    else  mParent->gotoPage(onLose);
    GameObserver::EndInstance();
    game=NULL;
  }
}

void StoryDuel::Render(){
  if(!game) return;
  game->Render();
}

string StoryDialog::safeAttribute(TiXmlElement* element, string attribute) {
  string s;
  if (element->Attribute(attribute.c_str())){
    s = element->Attribute(attribute.c_str());
  }
  return s;
}


StoryDialog::StoryDialog(TiXmlElement* root, StoryFlow * mParent):StoryPage(mParent), JGuiListener(), JGuiController(1,NULL) {

  for (TiXmlNode* node = root->FirstChild(); node; node = node->NextSibling()) {
		  TiXmlElement* element = node->ToElement();
		  if (element) {
			  if (strcmp(element->Value(), "text")==0) {
          string sX = safeAttribute(element, "x");
          float x = atof(sX.c_str());
          string sY = safeAttribute(element,"y");
          float y = atof(sY.c_str());
          string align = safeAttribute(element,"align");
          const char* textC = element->GetText();
          string text = textC;
          graphics.push_back(NEW StoryText(text,x,y,align));
			  }
        else if (strcmp(element->Value(), "img")==0) {
          string sX = safeAttribute(element,"x");
          float x = atof(sX.c_str());
          //special case to force center
          if (sX.compare("") == 0 ){
            x = -1;
          }
          string sY = safeAttribute(element,"y");
          float y = atof(sY.c_str());
          const char* imgC = element->GetText();
          string img = imgC;
          img = string("campaigns/").append(mParent->folder).append("/").append(img);
          graphics.push_back(NEW StoryImage(img,x,y));
			  }
        else if (strcmp(element->Value(), "answer")==0){
          string id = element->Attribute("goto");
          const char* answerC = element->GetText();
          string answer = answerC;
          int i = mObjects.size();
          StoryChoice * sc = NEW StoryChoice(id,answer,i,SCREEN_WIDTH/2, SCREEN_HEIGHT-20 - i *20 , (i==0));
          Add(sc);
        }else {
          //Error
        }
		  }
	  }
  this->mListener = this;

}

void StoryDialog::Update(float dt){
  JGuiController::Update(dt);
  for (size_t i = 0; i < graphics.size(); ++i){
    graphics[i]->Update(dt);
  }
}


void StoryDialog::Render() {
  for (size_t i = 0; i < graphics.size(); ++i){
    graphics[i]->Render();
  }
   JGuiController::Render();
}

void StoryDialog::ButtonPressed(int controllerid,int controlid)  {
  mParent->gotoPage(((StoryChoice *)mObjects[controlid])->pageId);
}

StoryDialog::~StoryDialog(){
    for (size_t i = 0; i < graphics.size(); ++i){
    delete(graphics[i]);
  }
}

StoryPage * StoryFlow::loadPage(TiXmlElement* element){
  TiXmlNode* typeNode = element->FirstChild("type");
  if (!typeNode) return NULL;
  StoryPage * result = NULL;
  const char* type = typeNode->ToElement()->GetText();
  if (strcmp(type, "duel")==0){
    result = NEW StoryDuel(element,this);
  }else{
    result = NEW StoryDialog(element,this);
  }
  return result;

}
//
bool StoryFlow::gotoPage(string id){
  currentPageId = id;
  return true;
}

bool StoryFlow::parse(string path)
{
	JFileSystem *fileSystem = JFileSystem::GetInstance();
	if (!fileSystem) return false;

	if (!fileSystem->OpenFile(path.c_str())) return false;

	int size = fileSystem->GetFileSize();
	char *xmlBuffer = new char[size];
	fileSystem->ReadFile(xmlBuffer, size);

	TiXmlDocument doc;
	doc.Parse(xmlBuffer);

	fileSystem->CloseFile();
	delete[] xmlBuffer;

	for (TiXmlNode* node = doc.FirstChild(); node; node = node->NextSibling()) {
		TiXmlElement* element = node->ToElement();
		if (element != NULL) {
			if (strcmp(element->Value(), "page")==0) {
        string id = element->Attribute("id");
				StoryPage * sp = loadPage(element);
        pages[id] = sp;
        if (!currentPageId.size()) gotoPage(id);
			}
      else {
        //Error
      }
		}
	}

	return true;
}

void StoryFlow::Update(float dt){
  pages[currentPageId]->Update(dt);

}

void StoryFlow::Render(){
  pages[currentPageId]->Render();

}

StoryFlow::~StoryFlow(){
  for (map<string,StoryPage*>::iterator i = pages.begin(); i != pages.end(); ++i){
    SAFE_DELETE(i->second);
  }
  pages.clear();
}
