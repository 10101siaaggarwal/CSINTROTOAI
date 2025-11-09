#ifndef STUDENTAI_H
#define STUDENTAI_H
#include "AI.h"
#include "Board.h"
#pragma once

//The following part should be completed by students.
//Students can modify anything except the class name and exisiting functions and varibles.
class StudentAI :public AI
{
public:
    Board board;
	StudentAI(int col, int row, int p);
	virtual Move GetMove(Move board);
};


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

 
class GTree{
    Node *root;
    int max_iter = 10000, max_seconds=60;

    public:
    GTree(Gstate *start_state, int max_iter);
    ~GTree();
    Node *select(double c); // select child to expand
    Node *select_best_child(); // select best 
    
    void grow_tree(int max_iter, double max_time_insecs);

    void advance_tree(const Move *move);
    unsigned int get_size() const;
    const Gstate *get_current_state() const;

    void get_states() const;
    const Move * generate(const Move move);
};

class Gstate {
public:
    // Implement these:
    virtual ~Gstate() = default;
    virtual queue<MCTS_move *> *actions_to_try() const = 0;
    virtual MCTS_state *next_state(const MCTS_move *move) const = 0;
    virtual double rollout() const = 0;
    virtual bool is_term() const = 0;
    virtual void print() const {
        cout << "Printing not implemented" << endl;
    }
    virtual bool player1_turn() const = 0;     // MCTS is for two-player games mostly -> (keeps win rate)
};

#endif //STUDENTAI_H
