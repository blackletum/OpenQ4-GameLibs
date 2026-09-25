// Lightweight world fixtures; decision functions below are taken verbatim from
// production by mp_bot_regressions.py. No second implementation of goal policy.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <vector>
#undef INFINITY
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #x); std::exit(1); } } while (0)
template<class T> T Min(T a, T b) { return std::min(a,b); }
template<class T> T Max(T a, T b) { return std::max(a,b); }
float Square(float x) { return x*x; }
float DEG2RAD(float x) { return x*0.01745329252f; }
struct idMath {
    static constexpr float INFINITY = std::numeric_limits<float>::infinity();
    static constexpr float FLOAT_EPSILON = 0.000001f;
    static float ClampFloat(float lo, float hi, float x) { return std::clamp(x,lo,hi); }
    static int ClampInt(int lo, int hi, int x) { return std::clamp(x,lo,hi); }
    static float Fabs(float x) { return std::fabs(x); }
    static float Sqrt(float x) { return std::sqrt(x); }
    static float Cos(float x) { return std::cos(x); }
    static float Ceil(float x) { return std::ceil(x); }
};
struct idVec3 {
    float x=0,y=0,z=0;
    idVec3() = default;
    idVec3(float a,float b,float c):x(a),y(b),z(c) {}
    idVec3 operator+(const idVec3& b) const { return {x+b.x,y+b.y,z+b.z}; }
    idVec3 operator-(const idVec3& b) const { return {x-b.x,y-b.y,z-b.z}; }
    idVec3 operator*(float f) const { return {x*f,y*f,z*f}; }
    float operator*(const idVec3& b) const { return x*b.x+y*b.y+z*b.z; }
    float LengthSqr() const { return *this * *this; }
    float Length() const { return std::sqrt(LengthSqr()); }
    float Normalize() { float n=Length(); if(n>0) *this=*this*(1/n); return n; }
    void Zero() { *this={}; }
    const char* ToString(int) const { return "fixture"; }
};
const idVec3 vec3_zero;
struct idBounds {
    idVec3 center;
    float radius=1;
    idVec3 GetCenter() const { return center; }
    float GetRadius() const { return radius; }
    float ShortestDistance(idVec3 p) const { return Max(0.0f,(p-center).Length()-radius); }
    idBounds operator+(idVec3 p) const { return {center+p,radius}; }
};
struct idMat3 { idVec3 forward{1,0,0}; idVec3 operator[](int) const { return forward; } };
template<class T> struct idList : std::vector<T> {
    int Num() const { return int(this->size()); }
    void SetGranularity(int) {}
    void Insert(T x,int i) { this->insert(this->begin()+i,x); }
    void RemoveIndex(int i) { this->erase(this->begin()+i); }
};
struct idStr : std::string { using std::string::string; };
struct Args {
    int health=0;
    bool fuse=false;
    int GetInt(const char*,const char*) const { return health; }
    bool GetBool(const char*,const char*) const { return fuse; }
};
struct Physics {
    idVec3 origin,velocity,gravity;
    float radius=1;
    const idVec3& GetOrigin() const { return origin; }
    const idVec3& GetLinearVelocity() const { return velocity; }
    const idVec3& GetGravity() const { return gravity; }
    idBounds GetAbsBounds() const { return {origin,radius}; }
    idBounds GetBounds() const { return {{},radius}; }
};
struct idEntity;
struct Link { idEntity* next=nullptr; idEntity* Next() const { return next; } };
struct idEntity {
    Physics physics;
    Args spawnArgs;
    Link spawnNode;
    int kind=0,instance=0,entityNumber=0;
    bool hidden=false;
    Physics* GetPhysics() { return &physics; }
    bool IsType(int type) const { return kind==type; }
    bool IsHidden() const { return hidden; }
    int GetInstance() const { return instance; }
};
struct idPlayer : idEntity {
    int health=100,visibilityCalls=0;
    float targetScore=1;
    bool visible=true,invisible=false;
    bool spectating=false;
    int spectator=-1;
    struct Inventory { int maxHealth=100; } inventory;
    idPlayer() { kind=1; }
    static int GetClassType() { return 1; }
    void GetViewPos(idVec3& eye,idMat3& axis) { eye=physics.origin; axis={}; }
    bool PowerUpActive(int) const { return invisible; }
};
struct idItem : idEntity {
    bool pickedUp=false;
    int pickedUpByClientNum=-1,claims=0;
    float utility=1;
    idItem() { kind=2; }
    static int GetClassType() { return 2; }
};
struct idProjectile : idEntity {
    float splash=0;
    idEntity* owner=nullptr;
    idProjectile() { kind=3; }
    static int GetClassType() { return 3; }
    idEntity* GetOwner() const { return owner; }
};
template<class T> struct idEntityPtr {
    T* ptr=nullptr;
    T* GetEntity() const { return ptr; }
    idEntityPtr& operator=(T* p) { ptr=p; return *this; }
};
enum { BOTGOAL_NONE,BOTGOAL_ROAM,BOTGOAL_ITEM,BOTGOAL_ENEMY,BOTGOAL_OBJECTIVE };
enum { BOTOBJ_NONE,BOTOBJ_CAPTURE,BOTOBJ_INTERCEPT,BOTOBJ_RETURN,BOTOBJ_ESCORT,BOTOBJ_FETCH };
enum { NAVTRAVEL_WALK,NAVTRAVEL_JUMP,POWERUP_INVISIBILITY,BUTTON_ATTACK,BOTCHAT_ITEM_DENIED };
const int MAX_CLIENTS=64;
enum { DEMO_NONE,DEMO_RECORDING,GAME_UNRELIABLE_RECORD_CLIENTNUM,GAME_UNRELIABLE_RECORD_AREAS_INSTANCE,GAME_UNRELIABLE_RECORD_AREAS };
using byte=unsigned char;
struct idBitMsg {
    int size=8;
    byte data[16]={};
    void Init(byte*,int) { size=0; }
    void WriteByte(int) { ++size; }
    void WriteChar(int) { ++size; }
    void WriteLong(int) { size+=4; }
    int GetSize() const { return size; }
    const byte* GetData() const { return data; }
};
struct idMsgQueue {
    int bytes=0,messages=0;
    int GetTotalSize() const { return bytes; }
    bool Add(const byte*,int size,bool) { bytes+=size; ++messages; return true; }
    void AddConcat(const byte*,int header,const byte* data,int size,bool b) { Add(data,size+header,b); }
};
struct Common {
    int warnings=0;
    template<class... T> void Warning(const char*,T...) { ++warnings; }
} commonObject;
Common* common=&commonObject;
// PRODUCTION_CONSTANTS
struct navCorner_t { idVec3 origin; int travelType=NAVTRAVEL_WALK; };
struct rvNavPath {
    std::vector<navCorner_t> corners;
    void Clear() { corners.clear(); }
    int Num() const { return int(corners.size()); }
    bool IsEmpty() const { return corners.empty(); }
    const navCorner_t& operator[](int i) const { return corners[i]; }
    void Append(idVec3 p,int type=NAVTRAVEL_WALK) { corners.push_back({p,type}); }
    float LengthFrom(idVec3 p) const {
        float result=0; for (auto c:corners) { result+=(c.origin-p).Length(); p=c.origin; } return result;
    }
};
struct Nav {
    int probes=0,searches=0;
    bool roam=false;
    std::vector<float> blocked;
    struct Node { int area=1; } node;
    bool FindPath(idVec3,idVec3 goal,rvNavPath& path) {
        ++searches; path.Clear();
        if(std::find(blocked.begin(),blocked.end(),goal.x)!=blocked.end()) return false;
        path.Append(goal); return true;
    }
    int FindNearestNode(idVec3 origin,float,bool) { ++probes; return origin.z<0 ? -1 : 0; }
    Node GetNode(int) const { return node; }
    bool RandomReachablePoint(idVec3,idVec3& target,rvNavPath* path) {
        if(!roam) return false; target={800,0,0}; path->Append(target); return true;
    }
} navMesh;
struct Command { int buttons=0; };
struct idGameLocal {
    int time=1000,numClients=0;
    idEntity* entities[64]={};
    Command commands[64];
    Command* usercmds=commands;
    Link spawnedEntities;
    int localClientNum=-1,demoState=DEMO_NONE,relayed=0;
    bool isRepeater=false;
    idMsgQueue unreliableMessages[MAX_CLIENTS+1];
    struct PVSHandle { int i=-1; } clientsPVS[MAX_CLIENTS];
    struct PVS {
        int queries=0;
        bool InCurrentPVS(PVSHandle,const int*,int) { ++queries; return true; }
    } pvs;
    void QueueUnreliableMessage(int,const idBitMsg&);
    void SendUnreliableMessage(const idBitMsg&,int);
    void SendUnreliableMessagePVS(const idBitMsg&,const idEntity*,int=-1,int=-1);
    void RepeaterUnreliableMessage(const idBitMsg&,int,const idBitMsg*) { ++relayed; }
    void RepeaterUnreliableMessagePVS(const idBitMsg&,const int*,int,const idBitMsg*) { ++relayed; }
    struct Random { float RandomFloat() { return 0.5f; } } random;
    template<class... T> void Printf(const char*,T...) {}
} gameLocal;
struct Debug { int GetInteger() const { return 0; } } bot_debug;
struct botObjective_t {
    int kind=BOTOBJ_NONE;
    idEntityPtr<idEntity> entity;
    idVec3 origin;
    float priority=0;
    bool holdPosition=false;
} fixtureObjective;
bool BotFindObjective(idPlayer*,botObjective_t& out) { out=fixtureObjective; return out.kind!=BOTOBJ_NONE; }
idVec3 BotObjectiveRouteOrigin(idPlayer*,const botObjective_t& o) { return o.origin; }
idStr BotReadableName(const Args&) { return "item"; }
const char* BotDisplayName(const idPlayer*) { return "player"; }
struct botItemCandidate_t { idItem* item; float utility,cheapScore; };
struct rvBot {
    idStr name="bot";
    int clientNum=0,pathCorner=0,pathTime=0,goalType=BOTGOAL_ROAM,goalGiveUpTime=0;
    int goalCommitUntil=0,goalProgressTime=1000,objectiveKind=BOTOBJ_NONE,repathFailures=0;
    int enemyPathTime=0,nextGoalSelectTime=0,holdUntil=0,noRouteSince=0;
    int traversalCorner=-1,stuckPathCorner=0;
    float goalUtility=0,goalBestDistance=idMath::INFINITY,stuckCornerDistance=idMath::INFINITY;
    bool objectiveHoldPosition=false,traversalEntered=false;
    idVec3 goalOrigin;
    idEntityPtr<idEntity> goalEntity;
    rvNavPath path;
    idEntityPtr<idPlayer> enemy,lastAttacker;
    int lastAttackerTime=0,enemyLastSeenTime=0,nextMistakeTime=0,onTargetTime=0;
    bool targetPickBest=true,aimBeliefValid=false,aimTrackValid=false;
    idVec3 enemyVisiblePoint,enemyLastSeenOrigin,enemyLastSeenVelocity;
    struct Traits {
        float itemFocus=0.5f,retreatHealth=0.3f,pursuit=0.5f,aggression=0.5f;
        float sightRange=3000,fov=180,targetStickiness=0.5f,reacquireMsec=600,targetSelection=1;
    } traits;
    int denied=0,abandoned=0;
    std::vector<idEntity*> avoided;
    void ResetTraversal() { traversalCorner=-1; traversalEntered=false; }
    bool AtObjectiveHoldPosition(idPlayer*) const { return false; }
    bool WantsHealth(idPlayer* p) const { return p->health < 30; }
    bool IsAvoided(const idEntity* e) const { return std::find(avoided.begin(),avoided.end(),e)!=avoided.end(); }
    void Avoid(idEntity* e,int) { if(e) { avoided.push_back(e); ++abandoned; } }
    void QueueChat(int,const char*,const char*,const char*) { ++denied; }
    idVec3 EnemyPursuitOrigin() const { return enemyLastSeenOrigin; }
    float ItemUtility(idPlayer*,idItem* item) const { return item->utility; }
    bool IsEnemy(idPlayer* self,idPlayer* other) const { return other && other!=self && other->health>0; }
    bool CanSee(idPlayer*,idEntity* e,idVec3* point) const {
        auto* p=static_cast<idPlayer*>(e); ++p->visibilityCalls;
        if(!p->visible) return false; *point=p->physics.origin; return true;
    }
    float TargetScore(idPlayer* p,float) const { return p->targetScore; }
    void AcquireEnemy(idPlayer*,idPlayer* p,bool) { enemy=p; }
    void RollMistake() {}
    void UpdateEnemy(idPlayer*);
    bool Repath(idPlayer*,const idVec3&,bool preserveProgress=false);
    float PathDistanceRemaining(const idVec3&) const;
    void AbandonGoal(int);
    idEntity* PickItemGoal(idPlayer*,rvNavPath&,float&);
    void UpdateGoal(idPlayer*);
};
struct Manager {
    bool bots[MAX_CLIENTS]={};
    bool IsBot(int i) const { return i>=0 && i<MAX_CLIENTS && bots[i]; }
    int GoalClaimCount(const rvBot*,idEntity* e) { return static_cast<idItem*>(e)->claims; }
} botManager;
struct botCombatProjectileImpact_t {
    bool terminating=false,hitsSelf=false;
    float time=0;
    idVec3 point;
    idEntity* hit=nullptr;
} fixtureImpact;
int impactSweeps=0;
bool BotCombatValidClientPlayer(idPlayer* p) { return p!=nullptr; }
bool BotCombatProjectileCanDamage(idProjectile*) { return true; }
bool BotCombatHostileProjectileOwner(idPlayer* self,idPlayer* owner) { return self!=owner; }
float BotCombatProjectileSplashRadius(idProjectile* p) { return p->splash; }
botCombatProjectileImpact_t BotCombatFindProjectileImpact(idProjectile*,idPlayer*,float,int) { ++impactSweeps; return fixtureImpact; }
float BotCombatProjectileDistanceAtTime(const idBounds& bounds,idVec3 sv,idVec3 p,idVec3 v,idVec3 g,float t,idVec3* point) {
    *point=p+v*t+g*(0.5f*t*t); return (bounds+sv*t).ShortestDistance(*point);
}
bool BotCombatSplashThreatensPlayer(idPlayer* p,idEntity*,idVec3 impact,float radius,float t) {
    return radius>0 && (p->physics.GetAbsBounds()+p->physics.velocity*t).ShortestDistance(impact)<=radius;
}
// PRODUCTION_FUNCTIONS

void Reset() {
    gameLocal=idGameLocal(); gameLocal.usercmds=gameLocal.commands;
    botManager=Manager(); commonObject.warnings=0;
    navMesh=Nav(); fixtureObjective={}; fixtureImpact={}; impactSweeps=0;
}
void GoalRoute(rvBot& b,idEntity* e,int type) {
    b.goalType=type; b.goalEntity=e; b.goalOrigin=e->physics.origin;
    b.path.Append({50,10,0}); b.path.Append(b.goalOrigin);
    b.goalBestDistance=b.path.LengthFrom({}); b.goalProgressTime=100;
    b.goalGiveUpTime=13000; b.goalUtility=20; b.pathTime=777;
}
void TestGoals() {
    Reset(); idPlayer self; idItem old; old.physics.origin={100,0,0};
    idEntity flag; flag.physics.origin={500,0,0};
    rvBot bot; GoalRoute(bot,&old,BOTGOAL_ITEM);
    fixtureObjective={BOTOBJ_CAPTURE,{},flag.physics.origin,100,false}; fixtureObjective.entity=&flag;
    navMesh.blocked={500};
    bot.UpdateGoal(&self);
    CHECK(bot.goalEntity.GetEntity()==&old && bot.goalType==BOTGOAL_ITEM);
    CHECK(bot.path.Num()==2 && bot.path[0].origin.y==10 && bot.pathTime==777);
    CHECK(bot.goalOrigin.x==100 && bot.goalProgressTime==100 && bot.goalGiveUpTime==13000);
    CHECK(bot.repathFailures==0);
    // A later reachable objective replaces the complete commitment together.
    gameLocal.time+=300; navMesh.blocked.clear(); bot.UpdateGoal(&self);
    CHECK(bot.goalEntity.GetEntity()==&flag && bot.goalType==BOTGOAL_OBJECTIVE);
    CHECK(bot.path.Num()==1 && bot.path[0].origin.x==500 && bot.goalOrigin.x==500);
    CHECK(bot.goalProgressTime==gameLocal.time && bot.goalGiveUpTime>gameLocal.time);
    // Failed refreshes preserve the existing path as well as progress.
    int progress=bot.goalProgressTime; navMesh.blocked={600};
    CHECK(!bot.Repath(&self,{600,0,0},true));
    CHECK(bot.path[0].origin.x==500 && bot.goalProgressTime==progress);
    // Repeated rebuilds of the SAME unreachable-in-practice goal must time out.
    bot.goalBestDistance=500; bot.goalProgressTime=100; bot.goalGiveUpTime=6000;
    for(int t=1600;t<=7000;t+=300) {
        gameLocal.time=t; bot.path.Clear(); bot.UpdateGoal(&self);
    }
    CHECK(bot.IsAvoided(&flag) && bot.abandoned==1);
    // Invalid pickups cannot leave an old route live when all searches fail.
    Reset(); rvBot invalid; GoalRoute(invalid,&old,BOTGOAL_ITEM); old.hidden=true;
    invalid.UpdateGoal(&self); CHECK(invalid.path.IsEmpty() && invalid.repathFailures==1);
    old.hidden=false;
    // A full inventory releases the old pickup's commitment immediately.
    Reset(); rvBot stocked; GoalRoute(stocked,&old,BOTGOAL_ITEM);
    stocked.goalUtility=100; stocked.goalCommitUntil=5000; old.utility=0;
    idPlayer foe; foe.physics.origin={400,0,0}; stocked.enemy=&foe;
    stocked.enemyLastSeenOrigin=foe.physics.origin; stocked.enemyLastSeenTime=gameLocal.time;
    stocked.UpdateGoal(&self);
    CHECK(stocked.goalType==BOTGOAL_ENEMY && stocked.goalEntity.GetEntity()==&foe);
    old.utility=1;
    // A fresh wander must replace the previous goal's distance baseline.
    Reset(); navMesh.roam=true; rvBot wander; wander.goalBestDistance=10;
    wander.UpdateGoal(&self); CHECK(wander.goalBestDistance==800);
    // Taking one's own pickup is not item denial. Another player's pickup is.
    for(int taker : {0,1,999}) {
        Reset(); gameLocal.numClients=2; old.pickedUp=true; old.pickedUpByClientNum=taker;
        rvBot picker; GoalRoute(picker,&old,BOTGOAL_ITEM); picker.UpdateGoal(&self);
        CHECK(picker.denied==(taker==0?0:1));
    }
    std::puts("Goal replacement, failed refresh, stalled recovery, wander baseline and pickup ownership: PASS");
}

void TestPerception() {
    // Exhaust all orderings with an occluded high-priority target, visible
    // alternatives and a current target. The optimal and nearest choices must
    // agree with exhaustive visibility even after pruning inferior candidates.
    idPlayer self;
    std::vector<int> order={0,1,2,3};
    int checked=0;
    do {
        for(bool best : {false,true}) for(int current=-1;current<4;++current) {
            Reset(); idPlayer players[4];
            const float distances[]={100,200,300,400}, scores[]={10,1,4,40};
            for(int i=0;i<4;++i) {
                players[i].physics.origin={distances[i],0,0}; players[i].targetScore=scores[i];
                players[i].visible=i!=1; players[i].entityNumber=i;
                gameLocal.entities[i]=&players[order[i]];
            }
            gameLocal.numClients=4; rvBot b; b.targetPickBest=best;
            if(current>=0) { b.enemy=&players[current]; b.enemyLastSeenTime=gameLocal.time; }
            idPlayer* expected=best ? &players[2] : &players[0];
            if(current>=0 && players[current].visible && expected!=&players[current] &&
               expected->targetScore>players[current].targetScore*0.75f) expected=&players[current];
            b.UpdateEnemy(&self);
            CHECK(b.enemy.GetEntity()==expected);
            CHECK(b.enemyVisiblePoint.x==expected->physics.origin.x);
            if(current>=0 && players[current].visible) CHECK(players[current].visibilityCalls==1);
            ++checked;
        }
    } while(std::next_permutation(order.begin(),order.end()));
    Reset(); idPlayer nearest,farther; nearest.physics.origin={100,0,0}; farther.physics.origin={500,0,0};
    nearest.targetScore=1; farther.targetScore=10; gameLocal.entities[0]=&nearest;
    gameLocal.entities[1]=&farther; gameLocal.numClients=2; rvBot pruned; pruned.UpdateEnemy(&self);
    CHECK(nearest.visibilityCalls==1 && farther.visibilityCalls==0);
    std::printf("Perception: %d exhaustive-order comparisons; inferior target uses zero visibility queries: PASS\n",checked);
}

void TestItems() {
    Reset(); idPlayer self; idItem items[80];
    for(int i=0;i<80;++i) {
        items[i].physics.origin={float(100+i*5),0,0}; items[i].utility=1;
        items[i].spawnNode.next=i+1<80 ? &items[i+1] : nullptr;
    }
    gameLocal.spawnedEntities.next=&items[0];
    rvBot b; rvNavPath path; float utility;
    CHECK(b.PickItemGoal(&self,path,utility)==&items[0]);
    CHECK(navMesh.probes==11 && navMesh.searches==10); // start + ten shortlisted goals
    // An unreachable high-utility item must not displace a valid shortlisted one.
    items[79].utility=100; items[79].physics.origin.z=-1;
    CHECK(b.PickItemGoal(&self,path,utility)==&items[0]);
    // Reorder the list: a later higher-ranked reachable candidate still wins.
    items[78].utility=10;
    CHECK(b.PickItemGoal(&self,path,utility)==&items[78]);
    std::puts("Item shortlist: 80 items need 11 snap queries instead of 81; invalid candidates do not evict valid ones: PASS");
}

void TestThreats() {
    Reset(); idPlayer self; idProjectile grenade;
    grenade.physics.origin={150,0,0}; grenade.splash=200; grenade.spawnArgs.fuse=true;
    gameLocal.spawnedEntities.next=&grenade;
    idProjectile* threat=nullptr; float when=0; idVec3 where;
    CHECK(BotCombatFindIncomingProjectileThreat(&self,0.85f,52,threat,when,where));
    CHECK(threat==&grenade && when==0.25f);
    // Explosion reach extends beyond the projectile's maximum travel.
    grenade.spawnArgs.fuse=false; grenade.physics.origin={1000,0,0}; grenade.physics.velocity={-1000,0,0};
    fixtureImpact.terminating=true; fixtureImpact.time=0.85f; fixtureImpact.point={150,0,0};
    CHECK(BotCombatFindIncomingProjectileThreat(&self,0.85f,52,threat,when,where));
    CHECK(threat==&grenade && when==0.85f);
    // Remote projectiles still never enter the expensive impact sweep.
    impactSweeps=0; grenade.physics.origin={3000,0,0};
    CHECK(!BotCombatFindIncomingProjectileThreat(&self,0.85f,52,threat,when,where));
    CHECK(impactSweeps==0 && threat==nullptr && when==0);
    std::puts("Threat broad phase: stationary fuse, distant rocket splash, cheap remote rejection: PASS");
}
void TestNetwork() {
    Reset(); idPlayer bot,observer,other; observer.spectating=true; observer.spectator=1;
    gameLocal.entities[1]=&bot; gameLocal.entities[2]=&observer; gameLocal.entities[3]=&other;
    gameLocal.numClients=4; botManager.bots[1]=true; gameLocal.demoState=DEMO_RECORDING;
    idBitMsg message;
    // Personal feedback for a bot still reaches its human spectator and demo.
    gameLocal.SendUnreliableMessage(message,1);
    CHECK(gameLocal.unreliableMessages[1].messages==0);
    CHECK(gameLocal.unreliableMessages[2].messages==1);
    CHECK(gameLocal.unreliableMessages[3].messages==0);
    CHECK(gameLocal.unreliableMessages[MAX_CLIENTS].messages==1);
    CHECK(gameLocal.relayed==1);
    // Broadcast reaches every remote human, even one not following anybody.
    gameLocal.SendUnreliableMessage(message,-1);
    CHECK(gameLocal.unreliableMessages[1].messages==0);
    CHECK(gameLocal.unreliableMessages[2].messages==2 && gameLocal.unreliableMessages[3].messages==1);
    // PVS queries themselves need not run for bots; recording stays active.
    gameLocal.clientsPVS[1].i=gameLocal.clientsPVS[2].i=gameLocal.clientsPVS[3].i=0;
    gameLocal.SendUnreliableMessagePVS(message,nullptr,0);
    CHECK(gameLocal.pvs.queries==2 && gameLocal.unreliableMessages[1].messages==0);
    CHECK(gameLocal.unreliableMessages[2].messages==3 && gameLocal.unreliableMessages[3].messages==2);
    CHECK(gameLocal.unreliableMessages[MAX_CLIENTS].messages==3 && gameLocal.relayed==3);
    // Direct queue callers cannot accumulate bot traffic either.
    for(int i=0;i<2000;++i) gameLocal.QueueUnreliableMessage(1,message);
    CHECK(gameLocal.unreliableMessages[1].GetTotalSize()==0 && commonObject.warnings==0);
    gameLocal.QueueUnreliableMessage(MAX_CLIENTS,message);
    CHECK(gameLocal.unreliableMessages[MAX_CLIENTS].messages==4);
    gameLocal.QueueUnreliableMessage(-1,message); gameLocal.QueueUnreliableMessage(MAX_CLIENTS+1,message);
    std::puts("Bot message queues: no accumulation or PVS work; spectator, broadcast, demo and repeater delivery retained: PASS");
}
int main(int argc,char** argv) {
    if(argc==1 || !std::strcmp(argv[1],"goals")) TestGoals();
    if(argc==1 || !std::strcmp(argv[1],"perception")) TestPerception();
    if(argc==1 || !std::strcmp(argv[1],"items")) TestItems();
    if(argc==1 || !std::strcmp(argv[1],"threats")) TestThreats();
    if(argc==1 || !std::strcmp(argv[1],"network")) TestNetwork();
}
