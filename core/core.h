#pragma once
#include "../payload.h"
#include <array>
#include <optional>
#include <vector>
namespace ryuugin {
enum class Side { Cam, Xanh };
enum class PieceType { Vuong, Tuong, Si, Ki, Xa, Long, Phuong, Binh };
enum class Facing { Up, Right, Down, Left };
struct Square { int row{},col{}; bool operator==(const Square&)const=default; int index()const; };
struct Piece { PieceType type; Side owner; Facing facing; bool operator==(const Piece&)const=default; };
using Stack=std::vector<Piece>;
using Board=std::array<Stack,81>;
using Hands=std::array<std::vector<PieceType>,2>;
struct Position { Board board; Side turn=Side::Cam; Hands reserve,prisoners; bool operator==(const Position&)const=default; };
enum class Kind { Single, Team, Rotate, Drop };
struct Move { Kind kind; std::optional<Square>src,dst; PieceType type; std::optional<Facing>facing; int rotation{}; ep::J notation; };
struct Result { bool terminal=false;std::string reason;std::optional<Side>winner,loser;bool operator==(const Result&)const=default; };
struct HistoryEntry { Position position; int count=1; };
struct GameState { Position position; int ply=0;std::vector<HistoryEntry> history;Result result;ep::J notation; };
struct Undo { GameState before;std::string after; };
Side other(Side);
void validate(const Position&);
ep::J encode(const Position&);
ep::J encode(const Move&);
ep::J encode(const GameState&);
Position decode_position(const ep::J&);
Move decode_move(const ep::J&);
GameState decode_state(const ep::J&,bool check_terminal=true);
std::string identity(const Position&);
bool attacked(const Position&,Square,Side);
bool in_check(const Position&,Side);
std::vector<Move> pseudo(const Position&);
std::vector<Move> legal(const Position&);
Undo make(GameState&,const Move&);
void unmake(GameState&,const Undo&);
void adjudicate(GameState&);
void max_plies(GameState&,int limit);
GameState initial();
std::string format_move(const Position&,const Move&); // canonical LPGF text, no invented checkmate marker
Move parse_move(const Position&,std::string); // live legal token, aliases and optional mate marker
static_assert(static_cast<int>(PieceType::Binh)==7);
static_assert(static_cast<int>(Facing::Left)==3);
static_assert(std::tuple_size_v<Board> ==81);
}
