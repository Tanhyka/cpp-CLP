#pragma once
#include "core.h"

namespace ryuugin::detail {
// Internal search boundary. Position must be validated, and move must come from
// legal(position). Never accept an untrusted wire action through this function.
// Uses the same transition as public transactional make; no JSON or history work.
void apply_generated(Position&, const Move&);
// Separate archival-record helpers. Never accepted by clp.move/1 or live make().
std::vector<Move> record_moves(const Position&,bool legacy_turnabout);
Undo make_record(GameState&,const Move&,bool legacy_turnabout);
std::string format_record(const Position&,const Move&);
struct Geometry { int row,col;bool sliding; };
std::vector<Geometry> geometry(PieceType,Facing);
}
