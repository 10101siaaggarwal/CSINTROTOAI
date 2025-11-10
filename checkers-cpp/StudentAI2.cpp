#include "StudentAI.h"
#include <random>
#include <chrono>
#include <cmath>

//The following part should be completed by students.
//The students can modify anything except the class name and exisiting functions and varibles.
StudentAI::StudentAI(int col,int row,int p)
	:AI(col, row, p)
{
    board = Board(col,row,p);
    board.initializeGame();
    player = 2;
}

Move StudentAI::GetMove(Move move)
{
    if (move.seq.empty())
    {
        player = 1;
    } else{
        board.makeMove(move,player == 1?2:1);
    }
    // vector<vector<Move> > moves = board.getAllPossibleMoves(player);
    // int i = rand() % (moves.size());
    // vector<Move> checker_moves = moves[i];
   //  int j = rand() % (checker_moves.size());
    // Move res = checker_moves[j];
    // board.makeMove(res,player);
    // return res;

    Gstate gstate(board, player);
    GTree gameTree(gstate);
    Move mov = * (gameTree.generate(move));

    board.makeMove(mov, player);
    return mov;
    //return select_best_child(Node.score)
}



Gstate::Gstate(Board board, int t):board(board), t(t)

bool Gstate::is_term(){
    return board.isWin(t) in {0,1,2};
}

bool Gstate::t1(){return t==1;}

Gstate Gstate::next_state(const Move move){
    
    board.makeMove(move, t);
    Board b = board;
    if(t==1)
    return Gstate(b,2 );

    return Gstate(b,1);
}

queue<Move > Gstate :: actions_to_try(){
    vector<vector<Move>> es = board.getAllPossibleMoves(board);
    std::queue q;
    for(int i=0; i<es.size(); i++){
        for(int j=0; j<i.size(), j++){
            q.push(es[i][j]);
        }
    }
    return q;
}

/*
class Node{
    bool term;
    unsigned int size;
    unsigned int number_of_simulations;
    double score;
    const Gstate * state; // current 
    const Move *move; // move to come from parent to current 
    vector<Node *> *children;
    Node * parent;
    queue < Move*> *totry;
    void backpropagate(double w, int n);
public:
    Node(const Gstate *state,Node *parent, const Move *move);

    ~Node();
    bool is_expanded() const;
    const Move *get_move() const;
    unsigned int get_size() const;
    bool is_term() const;

    void expand();
    void rollout();

    Node* select_best_child(double c) const;
    Node* advancetree(const Move *mov);
    const Gstate *get_current_state() const;

    void get_stats() const;
    double calculate_winrate(bool player1turn) const;

    
};
*/

Gstate gstate(board, player);
    GTree gameTree(gstate);
    Move mov = * (gameTree.generate(move));

Gstate::Gstate(Board board, int t):board(board), t(t)

virtual queue<MCTS_move *> *actions_to_try(){

}

virtual double rollout(){
    const Gstate* curr = this;
    const Gstate* prev_alloc = nullptr;
    std::mt19937 rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));

    while(true){
        //if(curr->is_term()){  
        //}

        queue<Move*>* q = curr->actions_to_try();

        if(!q || q->empty()){
            if(q){delete q;}
            double res = curr->player1_turn() ? 0.0 : 1.0;
            if(prev_alloc){delete prev_alloc;}
            return res;
        }

        std::vector<Move*> random_picks;
        random_picks.reserve(q->size());

        while(!q->empty()){
            random_picks.push_back(q->front());
            q->pop();
        }

        delete q;

        std::uniform_int_distribution<size_t> dist(0, pool.size()-1);
        size_t index = dist(rng);
        Move* chosen = random_picks[index];


        const Gstate* next = curr->next_state(chosen);

        for(size_t i = 0; i < random_picks.size(); ++i){
            if(i != index){delete random_picks[i];}
        }

        delete chosen;

        if(prev_alloc){delete prev_alloc;}

        prev_alloc=next;
        curr=next;
    }
}

bool Gstate::is_term(){
    return board.isWin(t) in {0,1,2};
}


Node::Node(const Gstate *state, Node *parent, const Move*move): state(state), parent(parent), move(move), score(0.0){
children = new vector<Node*>(10);
totry = state->actions_to_try();
term = state->is_term();
}


Node:: ~Node {
    delete state;
    delete move;
    for (int i=0; i<children.size();i++){
        delete children[i];
    }
    delete children;
    while(totry){
        delete totry->front();
        totry->pop();
    }
    delete totr;
}

bool Node::is_expanded() const{
    return term || totry->empty;
}

const Move* Node::get_move() const{
return move;
}

unsigned int Node::get_size() const{
    return size;
}

bool Node::is_term()const {return term;}

void Node::expand(){
if((!term) && ! is_expanded()){
    Move *next_move = totry ->front();
    totry->pop();
    Gstate *next_state = state->next_state(next_move);

    Node* nw = new Node(next_state, this, this);
    nw->rollout();
    children->push_back(nw);
}
if(term) rollout(); return;
if(is_expanded()) return;
}
// one child per call, not all child

void Node::rollout(){ // what?
backpropagate(state->rollout(), 1);
}

void Node::backpropagate(double result, int n=1){
    number_of_simulations +=n ;
    score+=n;
    if(parent) parent->backpropagate(result, n);
}

Node* Node::select_best_child(double c)const{
if (!children->empty()){
    if (children->size()==1) return children->at(0);
    else{
        double buct, uct = -1;
        Node* bchild = nullptr;
        for(int i = 0; i<children->size(); i++){

            if(children->at(i)->number_of_simulations ==0)continue;

            double wr = double(children->at(i)->score) / children[i]->number_of_simulations;
            if(!state->t1()) wr = 1-wr;

            if(c<=0) uct = wr;
            else {uct = wr + c*sqrt(log(double(number_of_simulations))/children[i]->number_of_simulations);}

            if (uct>buct) buct = uct; bchild = children->at(i);
        }
    }return bchild;
}
return nullptr;
}

Node* Node::advancetree(const Move *mov){
Node* next = nullptr;
for(int i = 0; i < children.size(); i++){
    if(*(children[i]->move) == *mov){
         next = children[i];
    }
    else delete children[i];
}

children->clear();

if (next) next->parent = nullptr;
else{Gstate *next_state = state->next_state(mov);
next = new Node(next_state, nullptr, nullptr);}

return next;
}

const Gstate * get_current_state() const{
return state;
}

void Node::get_stats() const{}

double Node::calculate_winrate(bool t1) const{
    int c = score/number_of_simulations;
    if(!t1){return 1-c;}
    return c;
}

GTree::GTree(Gstate *start_state, int max_iter, Move move):root(nullptr),max_iter(max_iter), max_seconds(60){
    root = new Node(start_state, nullptr, move); // root has no incoming move and no parent
}

GTree::~GTree(){
    delete root;
    root = nullptr;
 }

Node *GTree::select(double c){
    if(!root){return nullptr;}
    Node *node = root;
    while(node->is_expanded() && !node->is_term()){
        Node *next = node->select_best_child(c);
        if(!next){break;}
        node=next;
    }
    if(!node->is_term() && !node->is_expanded()){
        node->expand();
        Node *pick_next=node->select_best_child(c);
        if(pick_next){
            return pick_next;
        }
    }
    return node;
} // select child to expand

Node *GTree::select_best_child(){
    if(!root){return nullptr;}
    return root->select_best_child(1); // optimal c in mcts
}// select best 
    
void GTree::grow_tree(int max_iter, double max_time_insecs){
    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    const auto max_time = std::chrono::duration<double>(max_time_insecs);
    const double c_uct = std::sqrt(2.0);

    int iter = 0;
    while(iter<max_iter){
        if(max_time_insecs>0.0){
            auto elapse = clock::now()-start;
            if(elapse >= max_time){break;}
        }

        Node* leaf = select(c_uct);
        if(!leaf){break;}

        if(!leaf->is_term()){
            leaf->expand();
        }

        Node* simulate_at = leaf;

        if(!leaf->is_term() && leaf->is_expanded()){
            Node* child = leaf->select_best_child(c_uct);
            if(child){simulate_at = child;}
        }

        if(simulate_at){
            simulate_at->rollout();
        }
        ++iter;
    }
}

void GTree::advance_tree(const Move *move){
    if(!root){return;}
    Node* next = root->advancetree(move);
    if(next){
        root=next;
    }
}

unsigned int GTree::get_size() const{
    if(!root){return 0;}
    return root->get_size();
}

const Gstate *GTree::get_current_state() const{
    if(!root){return nullptr;}
    return root->get_current_state();
}

void GTree::get_states() const{
    if(root){
        root->get_stats();
    }
}

const Move *Gtree::generate(const Move emove){
    if (emove)
    this->advance_tree(emove);

    if (this->get_current_state()->is_term()) return nullptr;

    this->grow_tree(max_iter, max_seconds);

    Node* bchild = this->select_best_child();

    if(!bchild) return nullptr;

    const Move *betsmov = bchild->get_move();
    this->advance_tree(bestmov);
    return bestmov;
}

