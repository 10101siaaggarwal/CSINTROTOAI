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
Gstate::Gstate(const Board& b, int t) : board(b), t(t) {}

bool Gstate::is_term() const {
    int winner = board.isWin(t); // returns 0,1,2
    return winner == 0 || winner == 1 || winner == 2;
}

bool Gstate::t1() const { return t == 1; }

Gstate* Gstate::next_state(const Move* move) const {
    Board newBoard = board;
    newBoard.makeMove(*move, t);
    return new Gstate(newBoard, t == 1 ? 2 : 1);
}

std::queue<Move*>* Gstate::actions_to_try() const {
    auto* q = new std::queue<Move*>();
    std::vector<std::vector<Move>> allMoves = board.getAllPossibleMoves(t);

    for (auto& movesVec : allMoves) {
        for (auto& m : movesVec) {
            q->push(new Move(m));
        }
    }
    return q;
}

double Gstate::rollout() const {
    Gstate* curr = new Gstate(board, t);
    std::mt19937 rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));

    while (!curr->is_term()) {
        std::queue<Move*>* q = curr->actions_to_try();
        if (!q || q->empty()) break;

        std::vector<Move*> moves;
        while (!q->empty()) {
            moves.push_back(q->front());
            q->pop();
        }
        delete q;

        std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
        Move* chosen = moves[dist(rng)];
        Gstate* next = curr->next_state(chosen);

        for (Move* m : moves) if (m != chosen) delete m;
        delete chosen;
        delete curr;
        curr = next;
    }

    double result = curr->t1() ? 0.0 : 1.0;
    delete curr;
    return result;
}

Node::Node(Gstate* state, Node* parent, Move* move)
    : state(state), parent(parent), move(move), score(0.0), number_of_simulations(0)
{
    term = state->is_term();
    totry = state->actions_to_try();
}

Node::~Node() {
    for (Node* child : children) delete child;
    delete totry;
}

bool Node::is_expanded() const { return term || totry->empty(); }
bool Node::is_term() const { return term; }
const Move* Node::get_move() const { return move; }

void Node::expand() {
    if (term || is_expanded()) return;

    Move* next_move = totry->front();
    totry->pop();
    Gstate* next_state = state->next_state(next_move);

    Node* child = new Node(next_state, this, next_move);
    children.push_back(child);

    child->rollout();
}

void Node::rollout() {
    double result = state->rollout();
    backpropagate(result, 1);
}

void Node::backpropagate(double result, int n) {
    number_of_simulations += n;
    score += result;
    if (parent) parent->backpropagate(result, n);
}

Node* Node::select_best_child(double c) const {
    if (children.empty()) return nullptr;
    if (children.size() == 1) return children[0];

    double best = -1e9;
    Node* best_child = nullptr;

    for (Node* child : children) {
        if (child->number_of_simulations == 0) continue;
        double wr = double(child->score) / child->number_of_simulations;
        if (!state->t1()) wr = 1.0 - wr;

        double uct = (c <= 0) ? wr : wr + c * sqrt(log(double(number_of_simulations)) / child->number_of_simulations);
        if (uct > best) { best = uct; best_child = child; }
    }

    return best_child;
}

Node* Node::advancetree(const Move* mov) {
    Node* next = nullptr;
    for (Node* child : children) {
        if (*(child->move) == *mov) next = child;
        else delete child;
    }
    children.clear();

    if (next) { next->parent = nullptr; return next; }
    Gstate* next_state = state->next_state(mov);
    return new Node(next_state, nullptr, mov);
}

Gstate* Node::get_current_state() const { return state; }

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

GTree::GTree(Gstate* start_state, int max_iter, Move* move)
    : root(new Node(start_state, nullptr, move)), max_iter(max_iter), max_seconds(60.0)
{}

GTree::~GTree() { delete root; }

Node* GTree::select(double c) {
    Node* node = root;
    while (node->is_expanded() && !node->is_term()) {
        Node* next = node->select_best_child(c);
        if (!next) break;
        node = next;
    }

    if (!node->is_term() && !node->is_expanded()) {
        node->expand();
        Node* pick_next = node->select_best_child(c);
        if (pick_next) return pick_next;
    }

    return node;
}

Node* GTree::select_best_child() {
    if (!root) return nullptr;
    return root->select_best_child(1.0);
}

void GTree::grow_tree(int max_iter, double max_time_insecs) {
    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    const double c_uct = std::sqrt(2.0);

    for (int iter = 0; iter < max_iter; ++iter) {
        if (max_time_insecs > 0.0) {
            auto elapsed = clock::now() - start;
            if (std::chrono::duration<double>(elapsed).count() >= max_time_insecs) break;
        }

        Node* leaf = select(c_uct);
        if (!leaf) break;

        if (!leaf->is_term()) leaf->expand();

        Node* simulate_at = leaf;
        if (!leaf->is_term() && leaf->is_expanded()) {
            Node* child = leaf->select_best_child(c_uct);
            if (child) simulate_at = child;
        }

        if (simulate_at) simulate_at->rollout();
    }
}

void GTree::advance_tree(const Move* move) {
    if (!root) return;
    Node* next = root->advancetree(move);
    if (next) root = next;
}

const Move* GTree::generate(const Move* emove) {
    if (emove) advance_tree(emove);

    if (root->is_term()) return nullptr;

    grow_tree(max_iter, max_seconds);

    Node* best_child = select_best_child();
    if (!best_child) return nullptr;

    const Move* best_move = best_child->get_move();
    advance_tree(best_move);

    return best_move;
}