#include "StudentAI.h"
#include <random>
#include <chrono>
#include <cmath>
#include <vector>
#include <queue>
#include <limits>

// -------------------------------
// StudentAI (unchanged)
// -------------------------------
StudentAI::StudentAI(int col,int row,int p)
    : AI(col, row, p)
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
    }
    else
    {
        board.makeMove(move, (player == 1 ? 2 : 1));
    }

      std::vector<std::vector<Move>> moves = board.getAllPossibleMoves(player);
    int i = rand() % (moves.size());
    std::vector<Move> checker_moves = moves[i];
    int j = rand() % (checker_moves.size());
    Move res = checker_moves[j];
    board.makeMove(res, player);
    return res;
    
}

Gstate::Gstate(const Board& b, int turn) : board(b), t(turn) {}

bool Gstate::is_term() const {
    // Board::isWin(player) returns 0,1,2 for terminal (per your comment);
    // if it returns -1/other for "ongoing", this check will work as intended.
    int w = board.isWin(t);
    return (w == 0) || (w == 1) || (w == 2);
}

bool Gstate::player1_turn() const {
    return t == 1;
}

Gstate* Gstate::next_state(const Move move) const {
    Board nextBoard = board;
    nextBoard.makeMove(move, t);
    int nextTurn = (t == 1 ? 2 : 1);
    return new Gstate(nextBoard, nextTurn);
}

std::queue<Move>* Gstate::actions_to_try() const {
    auto* q = new std::queue<Move>();
    std::vector<std::vector<Move>> all = board.getAllPossibleMoves(t);
    for (auto& bucket : all) {
        for (auto& m : bucket) {
            q->push(m); // Move by value
        }
    }
    return q;
}

// Keep rollout as simple & fast as possible:
// - Uniform random playout using a single RNG
// - Moves by value (no Move* allocation)
// - States are heap-allocated; we delete as we go
double Gstate::rollout() const {
    // Fast RNG seed from steady_clock
    std::mt19937 rng(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));

    // Work on a copy (heap) and keep deleting as we advance
    Gstate* curr = new Gstate(board, t);

    while (!curr->is_term()) {
        std::queue<Move>* q = curr->actions_to_try();
        if (!q || q->empty()) {
            if (q) delete q;
            break; // no legal moves; terminal should be true anyway
        }

        // Collect to a small vector for O(1) random access
        std::vector<Move> pool;
        pool.reserve(q->size());
        while (!q->empty()) { pool.push_back(q->front()); q->pop(); }
        delete q;

        std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
        const Move& chosen = pool[dist(rng)];

        // Step to next state; delete old
        Gstate* next = curr->next_state(chosen);
        delete curr;
        curr = next;
    }

    // Very simple outcome proxy (same as your earlier version):
    // if it's player1's turn at terminal, we say 0.0, else 1.0.
    // You can replace this with your actual winner decoding if desired.
    double result = curr->player1_turn() ? 0.0 : 1.0;
    delete curr;
    return result;
}

// -------------------------------
// Node implementation
// -------------------------------

Node::Node(const Gstate *state, Node *parent, const Move move)
    : term(false),
      size(1),
      number_of_simulations(0),
      score(0.0),
      state(state),
      move(move),
      children(new std::vector<Node*>()),
      parent(parent),
      totry(nullptr)
{
    // Assume ownership of 'state' lives with this node (including root).
    term = state->is_term();
    // Queue of actions to try comes from the game state
    totry = state->actions_to_try(); // std::queue<Move>* (by value Moves)
}

Node::~Node()
{
    // Delete all children
    if (children)
    {
        for (Node* ch : *children) delete ch;
        delete children;
        children = nullptr;
    }
    // Delete pending actions
    if (totry)
    {
        delete totry;
        totry = nullptr;
    }
    // Delete owned state
    delete state;
}

bool Node::is_expanded() const
{
    // A terminal node is trivially "expanded";
    // otherwise expanded iff there are no more untried actions.
    return term || (totry && totry->empty());
}

const Move Node::get_move() const
{
    return move; // by value
}

unsigned int Node::get_size() const
{
    // Recompute size on demand: 1 + sum(children)
    unsigned int s = 1U;
    if (children)
    {
        for (const Node* ch : *children) s += ch->get_size();
    }
    return s;
}

bool Node::is_term() const
{
    return term;
}

void Node::expand()
{
    if (term) return;
    if (!totry || totry->empty()) return;

    // Pop one untried action and create a single child
    Move m = totry->front();
    totry->pop();

    const Gstate* next = state->next_state(m); // returns a newly allocated state
    Node* child = new Node(next, this, m);
    children->push_back(child);
}

void Node::backpropagate(double w, int n)
{
    number_of_simulations += static_cast<unsigned int>(n);
    score += w;
    if (parent) parent->backpropagate(w, n);
}

void Node::rollout()
{
    // One playout from this state's position.
    // We assume state->rollout() returns a score in [0,1] from Player1's perspective.
    double result = state->rollout();
    backpropagate(result, 1);
}

Node* Node::select_best_child(double c) const
{
    if (!children || children->empty()) return nullptr;
    if (children->size() == 1) return (*children)[0];

    // UCT selection
    double best_score = -std::numeric_limits<double>::infinity();
    Node* best_child = nullptr;

    double ln_parent = (number_of_simulations > 0)
        ? std::log(static_cast<double>(number_of_simulations))
        : 0.0;

    for (Node* ch : *children)
    {
        if (!ch || ch->number_of_simulations == 0) {
            // Prefer unvisited child to encourage exploration
            return ch;
        }

        // Winrate from Player1 perspective
        double wr = ch->score / static_cast<double>(ch->number_of_simulations);

        // If it's NOT Player1's turn at this node's state, flip perspective
        // so the UCT maximizes the effective value from the player-to-move.
        if (!state->player1_turn()) wr = 1.0 - wr;

        double uct = (c <= 0.0)
                   ? wr
                   : wr + c * std::sqrt( ln_parent / static_cast<double>(ch->number_of_simulations) );

        if (uct > best_score) {
            best_score = uct;
            best_child = ch;
        }
    }
    return best_child;
}

Node* Node::advancetree(const Move mov)
{
    // Try to reuse an existing child that matches the external move;
    // delete all other children to keep the tree compact.
    Node* keep = nullptr;

    if (children)
    {
        for (Node* ch : *children)
        {
            if (ch->move == mov) {
                keep = ch;
            } else {
                delete ch;
            }
        }
        children->clear();
    }

    if (keep)
    {
        // Detach it from current node to make it the new root.
        keep->parent = nullptr;
        return keep;
    }

    // If the move wasn't among the children, we must create the next node from scratch.
    Gstate* next = state->next_state(mov);
    Node* fresh = new Node(next, nullptr, mov);
    return fresh;
}

const Gstate* Node::get_current_state() const
{
    return state;
}

void Node::get_stats() const
{
    // (No-op stub; left empty per your request to avoid extra output/work.)
}

double Node::calculate_winrate(bool player1turn) const
{
    if (number_of_simulations == 0) return 0.0;
    double wr = score / static_cast<double>(number_of_simulations);
    return player1turn ? wr : (1.0 - wr);
}

// -------------------------------
// GTree implementation
// -------------------------------

GTree::GTree(Gstate *start_state, int max_iter, Move move)
    : root(nullptr), max_iter(max_iter), max_seconds(60)
{
    // Root has no parent; we pass ownership of start_state to the root node.
    root = new Node(start_state, nullptr, move);
}

GTree::~GTree()
{
    delete root;
    root = nullptr;
}

Node* GTree::select(double c)
{
    if (!root) return nullptr;

    Node* node = root;
    // Descend while fully expanded and non-terminal
    while (node->is_expanded() && !node->is_term())
    {
        Node* next = node->select_best_child(c);
        if (!next) break;
        node = next;
    }

    // If we reached a non-terminal, not-yet-expanded node, expand one child
    if (!node->is_term() && !node->is_expanded())
    {
        node->expand();
        Node* pick_next = node->select_best_child(c);
        if (pick_next) return pick_next;
    }

    return node;
}

Node* GTree::select_best_child()
{
    if (!root) return nullptr;
    // c = 0 -> pure exploitation when picking the final action
    return root->select_best_child(0.0);
}

void GTree::grow_tree(int max_iter_param, double max_time_insecs)
{
    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    const double c_uct = std::sqrt(2.0);

    int iters = (max_iter_param > 0 ? max_iter_param : max_iter);

    for (int iter = 0; iter < iters; ++iter)
    {
        if (max_time_insecs > 0.0)
        {
            auto elapsed = clock::now() - start;
            if (std::chrono::duration<double>(elapsed).count() >= max_time_insecs) break;
        }

        Node* leaf = select(c_uct);
        if (!leaf) break;

        if (!leaf->is_term()) leaf->expand();

        Node* simulate_at = leaf;
        if (!leaf->is_term() && leaf->is_expanded())
        {
            Node* child = leaf->select_best_child(c_uct);
            if (child) simulate_at = child;
        }

        if (simulate_at) simulate_at->rollout();
    }
}

void GTree::advance_tree(const Move *move)
{
    if (!root || !move) return;
    Node* next = root->advancetree(*move);
    if (next)
    {
        delete root;
        root = next;
    }
}

unsigned int GTree::get_size() const
{
    if (!root) return 0 ;
    return root->get_size();
}

const Gstate* GTree::get_current_state() const
{
    if (!root) return nullptr;
    return root->get_current_state();
}

void GTree::get_states() const
{
    if (root) root->get_stats();
}

const Move* GTree::generate(const Move move)
{
    
    advance_tree(&move);

    if (root->is_term()) return nullptr;

    
    grow_tree(max_iter, static_cast<double>(max_seconds));

     
    Node* best = select_best_child();
    if (!best) return nullptr;

     
    static Move chosen;
    chosen = best->get_move();

     
    advance_tree(&chosen);
    return &chosen;
}
