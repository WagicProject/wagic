#ifndef TASK_H
#define TASK_H

#include <vector>

class GameObserver;

// Task type constant

#define TASK_BASIC 'B'

#define TASK_WIN_AGAINST 'W'
#define TASK_SLAUGHTER 'S'
#define TASK_DELAY 'D'
#define TASK_IMMORTAL 'I'
#define TASK_MASSIVE_BURIAL 'M'
#define TASK_WISDOM 'O'
#define TASK_PACIFISM 'P'
#define TASKS_ALL "WSDIMOP"

#define ITEM_SEPARATOR "|"

#define COMMON_ATTRIBS_COUNT 7

class Task
{
protected:
    int reward; // TODO: Complex rewards. Be consistent with other planned modes with rewards.
    int opponent;
    bool accepted;
    char type;
    int expiresIn;
    string description;
    string opponentName;
    vector<string> persistentAttribs; // persistentAttributes

    void storeCommonAttribs();
    int restoreCommonAttribs();
    string getOpponentName();
    virtual void storeCustomAttribs();
    virtual void restoreCustomAttribs();
    virtual void randomize();

    virtual int computeReward() = 0;

public:
    // variable to store and method to obtain names of AI decks
    //!! Todo: This should _really_ be handled elsewhere (dedicated class?)
    static vector<string> sAIDeckNames;
    static void LoadAIDeckNames();
    static int getAIDeckCount();
    static string getAIDeckName(int id);
    // End of AI deck buffering code

    Task(char _type = ' ');
    virtual ~Task(){};

    static Task* createFromStr(const string params, bool rand = false);
    virtual string toString();
    string getDesc();
    virtual string createDesc() = 0;
    virtual string getShortDesc() = 0;
    int getExpiration();
    int getReward();
    virtual bool isDone(GameObserver* observer, GameApp * _app) = 0;
    bool isExpired();
    void setExpiration(int _expiresIn);
    void passOneDay();
};

class TaskList
{
protected:
    string fileName;
    float vPos;
    float mElapsed;
    int mState;
    JQuad * mBg[9];
    JTexture * mBgTex;
    float sH, sW;

public:
    vector<Task*> tasks;

    enum
    {
        TASKS_IN,
        TASKS_ACTIVE,
        TASKS_OUT,
        TASKS_INACTIVE,
    };

    TaskList(string _fileName = "");
    int load(string _fileName = "");
    int save(string _fileName = "");
    int getState()
    {
        return mState;
    }
    ;
    void addTask(string params, bool rand = false);
    void addTask(Task *task);
    void addRandomTask(int diff = 100);
    void removeTask(Task *task);
    void passOneDay();
    void getDoneTasks(GameObserver* observer, GameApp * _app, vector<Task*>* result);
    int getTaskCount();

    void Start();
    void End();

    void Update(float dt);
    void Render();
    //!!virtual void ButtonPressed(int controllerId, int controlId);

    ~TaskList();
};

class TaskWinAgainst: public Task
{
protected:
    virtual int computeReward();
public:
    TaskWinAgainst(int _opponent = 0);
    virtual string createDesc();
    virtual string getShortDesc();
    virtual bool isDone(GameObserver* observer, GameApp * _app);
};

class TaskSlaughter: public TaskWinAgainst
{
protected:
    int targetLife;
    virtual int computeReward();
public:
    TaskSlaughter(int _opponent = 0, int _targetLife = -15);
    virtual string createDesc();
    virtual string getShortDesc();
    virtual bool isDone(GameObserver* observer, GameApp * _app);
    virtual void storeCustomAttribs();
    virtual void restoreCustomAttribs();
    virtual void randomize();
};

class TaskDelay: public TaskWinAgainst
{
protected:
    int turn;
    bool afterTurn;
    virtual int computeReward();
public:
    TaskDelay(int _opponent = 0, int _turn = 20);
    virtual string createDesc();
    virtual string getShortDesc();
    virtual bool isDone(GameObserver* observer, GameApp * _app);
    virtual void storeCustomAttribs();
    virtual void restoreCustomAttribs();
    virtual void randomize();
};

class TaskImmortal: public Task
{
protected:
    int targetLife;
    int level;
    virtual int computeReward();
public:
    TaskImmortal(int _targetLife = 20);

    virtual string createDesc();
    virtual string getShortDesc();
    virtual bool isDone(GameObserver* observer, GameApp * _app);
    virtual void storeCustomAttribs();
    virtual void restoreCustomAttribs();
    virtual void randomize();
};

class TaskMassiveBurial: public Task
{
protected:
    int color;
    int bodyCount;
    virtual int computeReward();
public:
    TaskMassiveBurial(int _color = 0, int _bodyCount = 0);

    virtual string createDesc();
    virtual string getShortDesc();
    virtual bool isDone(GameObserver* observer, GameApp * _app);
    virtual void storeCustomAttribs();
    virtual void restoreCustomAttribs();
    virtual void randomize();
};

class TaskWisdom: public Task
{
protected:
    int color;
    int cardCount;
    virtual int computeReward();
public:
    TaskWisdom(int _color = 0, int _cardCount = 0);

    virtual string createDesc();
    virtual string getShortDesc();
    virtual bool isDone(GameObserver* observer, GameApp * _app);
    virtual void storeCustomAttribs();
    virtual void restoreCustomAttribs();
    virtual void randomize();
};

class TaskPacifism: public Task
{
protected:
    virtual int computeReward();
    int lifeSlashCardMin;
public:
    TaskPacifism(int _lifeSlashCardMin = 0);

    virtual string createDesc();
    virtual string getShortDesc();
    virtual bool isDone(GameObserver* observer, GameApp * _app);
    virtual void storeCustomAttribs();
    virtual void restoreCustomAttribs();
    virtual void randomize();
};

/* ------------ Task template ------------ 

 class TaskXX : public Task {
 protected:
 virtual int computeReward();
 public:
 TaskXX();

 virtual string createDesc();
 virtual string getShortDesc();
 virtual bool isDone(GameObserver* observer, GameApp * _app);
 virtual void storeCustomAttribs();
 virtual void restoreCustomAttribs();
 virtual void randomize();
 };
 */

#endif
