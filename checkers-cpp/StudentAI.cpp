#include "StudentAI.h"
#include <random>

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


}


class Node{
    // bool term
    // unsigned int size
    unsigned int number_of_simulations;
    double score;
    const Gstate * state; // current 
    const Gmov *move; // move to come from parent to current 
    vector<Node *> *children;
    Node * parent;
    queue < Gmov*> *totry; // why
    // void backpropagate(double, )

    public:


    Node(const Gstate *state,Node *parent, const Gmov *move);

    ~Node();
    bool is_expanded() const;
    const Gmov *get_move() const;
    unsigned int get_size() const;
    bool is_term() const;

    void expand();
    void rollout();

    Node* select_best_child(double c) const;
    Node* advancetree(const Gmov *mov);
    const Gstate *get_current_state() const;

    void get_stats() const;
    double calculate_winrate(bool player1turn) const;

    
};

 

class GTree{
    Node *root;
    int max_iter = 10000, max_seconds=60;

    public:
    GTree(Gstate *start_state, int max_iter);
    ~GTree();
    Node* select(double c=? ); // select child to expand
    Node* select_best_child(); // select best 
    
    void grow_tree(int max_iter, double_max_time_insecs);

    void advance_tree(const Gmove *move);
    unsigned int get_size() const;
    const Gstate *get_current_state() const;

    void get_states() const;

};


class 