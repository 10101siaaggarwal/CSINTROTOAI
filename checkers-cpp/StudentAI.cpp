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
    vector<vector<Move> > moves = board.getAllPossibleMoves(player);
    int i = rand() % (moves.size());
    vector<Move> checker_moves = moves[i];
    int j = rand() % (checker_moves.size());
    Move res = checker_moves[j];
    board.makeMove(res,player);
    return res; 
    //return select_best_child(Node.score)
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

GTree::GTree(Gstate *start_state, int max_iter):root(nullptr),max_iter(max_iter), max_seconds(60){
    root = new Node(start_state, nullptr, nullptr); // root has no incoming move and no parent
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
    return root->select_best_child(0.0); // optimal c in mcts
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
            Node* child = leaf->select_best_child(0.0);
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
