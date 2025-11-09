#ifndef STUDENTAI_H
#define STUDENTAI_H
#include "AI.h"
#include "Board.h"
#pragma once

#include <queue>

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
    const Move  move; // move to come from parent to current 
    vector<Node *> *children;
    Node * parent;
    std::queue <Move > *totry;
    void backpropagate(double w, int n);
public:
    Node(const Gstate *state,Node *parent, const Move  move);
    ~Node();
    bool is_expanded() const;
    const Move  get_move() const;
    unsigned int get_size() const;
    bool is_term() const;

    void expand();
    void rollout();

    Node* select_best_child(double c) const;
    Node* advancetree(const Move  mov);
    const Gstate *get_current_state() const;

    void get_stats() const;
    double calculate_winrate(bool player1turn) const;

};

 
class GTree{
    Node *root;
    int max_iter = 10000, max_seconds=60;

    public:
    GTree(Gstate *start_state, int max_iter, Move move);
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

class State


 
// board, t , term, win
Gstate(Board board, int t);
bool is_term();
#endif //STUDENTAI_H

queue<Move> actions_to_try(); // q of *m
bool is_term();
Gstate next_state(const Move move);
bool t1()