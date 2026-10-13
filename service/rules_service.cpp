#include "../core/core_search.h"
#include <cstring>
#include <algorithm>
#include <chrono>
#include <unordered_map>

namespace {
using namespace ryuugin;using ep::J;
const std::array<std::string,8>types{"VUONG","TUONG","SI","KI","XA","LONG","PHUONG","BINH"};
J text(const std::string&s){J j;j.type=J::String;j.str=s;return j;}
J number(int64_t v){J j;j.type=J::Int;j.number=v;return j;}
J boolean(bool v){J j;j.type=J::Bool;j.number=v;return j;}
J array(){J j;j.type=J::Array;return j;}
J object(std::initializer_list<std::pair<const std::string,J>>v){J j;j.type=J::Object;j.obj=v;return j;}
Side side(const std::string&s){ep::require(ep::side(s),"side");return s=="CAM"?Side::Cam:Side::Xanh;}
PieceType type(const std::string&s){auto it=std::find(types.begin(),types.end(),s);ep::require(it!=types.end(),"type");return PieceType(it-types.begin());}
bool same(const Move&a,const Move&b){return a.kind==b.kind&&a.src==b.src&&a.dst==b.dst&&a.type==b.type&&a.facing==b.facing&&a.rotation==b.rotation;}
Move record_move(J j){
 if(j.at("schema").str=="clp.record_move/1"){
  ep::require(j.at("rotate_after").integer(0,3)==2,"record schema only180");j.obj["schema"]=text("clp.move/1");j.obj["rotate_after"]=number(j.at("kind").str=="rotate"?1:0);auto m=decode_move(j);ep::require(m.kind!=Kind::Drop&&m.type!=PieceType::Vuong&&m.type!=PieceType::Long&&m.type!=PieceType::Phuong,"record180 type/kind");m.rotation=2;return m;
 }return decode_move(j);
}
J record_wire(Move m){if(m.rotation!=2)return encode(m);m.rotation=1;auto j=encode(m);j.obj["schema"]=text("clp.record_move/1");j.obj["rotate_after"]=number(2);return j;}
std::optional<Square>read_square(const J&j){if(j.type==J::Null)return {};ep::square(j);return Square{int(j.arr[0].number),int(j.arr[1].number)};}
J editor(const J&position,bool fill,Side holder){
 // Editor staging may lack kings or exceed physical inventory; no live legality
 // is claimed here. Validate value domains before counting/modifying arrays.
 position.keys({"board","to_move","reserve","prisoners"});side(position.at("to_move").text());
 const auto&board=position.at("board");ep::require(board.type==J::Array&&board.arr.size()==81,"editor board81");
 std::array<int,8>counts{};std::array<int,2>kings{};
 for(const auto&st:board.arr){ep::require(st.type==J::Array&&st.arr.size()<=40,"editor cell cap");for(const auto&q:st.arr){q.keys({"type","owner","facing"});auto t=type(q.at("type").text());auto s=side(q.at("owner").text());q.at("facing").integer(0,3);++counts[int(t)];if(t==PieceType::Vuong)++kings[int(s)];}}
 for(auto hand:{"reserve","prisoners"}){position.at(hand).keys({"CAM","XANH"});for(auto name:{"CAM","XANH"}){const auto&v=position.at(hand).at(name);ep::require(v.type==J::Array&&v.arr.size()<=80,"editor hand cap");for(const auto&t:v.arr)++counts[int(type(t.text()))];}}
 const std::array<int,8>max{2,4,4,4,4,2,2,18};bool overflow=kings[0]>1||kings[1]>1;auto totals=object({});for(int n=0;n<8;++n){totals.obj[types[n]]=number(counts[n]);overflow|=counts[n]>max[n];}
 auto result=object({{"counts",totals},{"kings",object({{"CAM",number(kings[0])},{"XANH",number(kings[1])}})},{"overflow",boolean(overflow)},{"position",position},{"filled",boolean(false)}});
 if(fill&&!overflow){auto&stock=result.obj["position"].obj["prisoners"].obj[holder==Side::Cam?"CAM":"XANH"].arr;for(int n=1;n<8;++n)for(int k=counts[n];k<max[n];++k)stock.push_back(text(types[n]));result.obj["filled"]=boolean(true);}
 return result;
}
J handle(const J&r){
 auto command=r.at("command").text();
 if(command=="metadata"){
  auto list=array();std::string codes="VTSKXLPB";const std::array<int,8>max{2,4,4,4,4,2,2,18};
  for(int i=0;i<8;++i)list.arr.push_back(object({{"type",text(types[i])},{"code",text(codes.substr(size_t(i),1))},{"rotatable",boolean(i!=0&&i!=5&&i!=6)},{"physical_total",number(max[i])}}));
  return object({{"api",text("ryuugin.rules/1")},{"types",list},{"board_size",number(9)},{"max_stack",number(38)},{"max_pieces",number(40)},{"initial_facing",object({{"CAM",number(2)},{"XANH",number(0)}})}});
 }
 if(command=="initial")return encode(initial());
 if(command=="editor_counts"||command=="editor_fill")return editor(r.at("position"),command=="editor_fill",r.obj.contains("side")?side(r.at("side").text()):Side::Cam);
 if(command=="geometry"){
  auto list=array();auto t=type(r.at("type").text());for(auto v:detail::geometry(t,Facing(r.at("facing").integer(0,3))))list.arr.push_back(object({{"row",number(v.row)},{"col",number(v.col)},{"kind",text(v.sliding?"slide":t==PieceType::Ki?"jump":"step")}}));return list;
 }
 if(command=="resolve_record"){
  auto src=read_square(r.at("src")),dst=read_square(r.at("dst"));ep::require(dst.has_value(),"record destination");std::vector<Move>unique;
  const auto&v=r.at("moves");ep::require(v.type==J::Array&&v.arr.size()<=4096,"candidate cap");for(const auto&j:v.arr){auto m=record_move(j);if(std::none_of(unique.begin(),unique.end(),[&](auto&a){return same(a,m);}))unique.push_back(m);}
  std::vector<Move>matches;for(auto m:unique)if(src?m.src==src:(m.kind==Kind::Rotate&&m.src==dst))matches.push_back(m);
  if(src||!matches.empty())return matches.size()==1?record_wire(matches[0]):J{};
  return unique.size()==1?record_wire(unique[0]):J{};
 }
 if(command=="position_identity"||command=="position_legal"||command=="record_moves"||command=="parse_record"||command=="in_check"||command=="format"||command=="transition"||command=="base"){
  auto p=decode_position(r.at("position"));
  if(command=="position_identity")return text(identity(p));
  if(command=="in_check")return boolean(in_check(p,side(r.at("side").text())));
  if(command=="parse_record"){
   auto src=read_square(r.at("src")),dst=read_square(r.at("dst"));ep::require(dst.has_value(),"record destination");
   const auto code=r.at("code").text();const std::string codes="VTSKXLPB";ep::require(code.size()==1&&codes.find(code)!=std::string::npos,"record piece code");auto t=PieceType(codes.find(code));
   ep::require(r.at("team").type==J::Bool&&r.at("drop").type==J::Bool&&r.at("txt_semantics").type==J::Bool&&r.at("legacy_turnabout").type==J::Bool,"record flags");
   bool team=r.at("team").number!=0,drop=r.at("drop").number!=0,txt=r.at("txt_semantics").number!=0,legacy=r.at("legacy_turnabout").number!=0;
   auto direction=r.at("direction").text();int facing=p.turn==Side::Cam?2:0;
   if(!direction.empty()){
    if(direction=="^"||direction=="˄")facing=0;else if(direction==">")facing=1;else if(direction=="v"||direction=="V"||direction=="˅")facing=2;else if(direction=="<")facing=3;else throw ep::Error("BAD_PAYLOAD","record direction");
   }
   std::vector<Move>matches;
   for(auto m:detail::record_moves(p,legacy)){
    if(m.type!=t||(src&&m.src!=src))continue;
    if(drop?m.kind!=Kind::Drop:(team?m.kind!=Kind::Team:(m.kind!=Kind::Single&&m.kind!=Kind::Rotate)))continue;
    if((m.kind==Kind::Rotate?m.src:m.dst)!=dst)continue;
    if(t!=PieceType::Vuong&&t!=PieceType::Long&&t!=PieceType::Phuong){
     if(m.kind==Kind::Drop){if(int(*m.facing)!=facing)continue;}
     else if(txt&&direction.empty()){if(m.rotation!=0)continue;}
     else if((int(p.board[m.src->index()].back().facing)+m.rotation)%4!=facing)continue;
    }matches.push_back(m);
   }
   if(!src){auto it=std::find_if(matches.begin(),matches.end(),[&](const Move&m){return m.kind==Kind::Rotate&&m.src==dst;});if(it!=matches.end())return record_wire(*it);}
   ep::require(matches.size()==1,"record illegal/ambiguous; explicit source required");return record_wire(matches[0]);
  }
  if(command=="position_legal"||command=="record_moves"){
   auto list=array();bool legacy=command=="record_moves"&&r.at("legacy_turnabout").type==J::Bool&&r.at("legacy_turnabout").number;
   for(auto m:detail::record_moves(p,legacy))list.arr.push_back(record_wire(m));return list;
  }
  if(command=="format")return text(detail::format_record(p,record_move(r.at("move"))));
  GameState g;g.position=p;g.history={{p,1}};adjudicate(g);
  if(command=="base")return encode(g);
  auto m=record_move(r.at("move"));bool legacy=r.obj.contains("legacy_turnabout")&&r.at("legacy_turnabout").type==J::Bool&&r.at("legacy_turnabout").number;
  if(m.rotation==2)detail::make_record(g,m,legacy);else make(g,m);return encode(g.position);
 }
 auto g=decode_state(r.at("state"));
 if(command=="roundtrip")return encode(g);
 if(command=="apply"||command=="apply_record"){
  auto m=command=="apply_record"?record_move(r.at("move")):decode_move(r.at("move"));
  if(command=="apply_record")detail::make_record(g,m,r.at("legacy_turnabout").type==J::Bool&&r.at("legacy_turnabout").number);else make(g,m);
  return encode(g);
 }
 if(command=="status"){adjudicate(g);return encode(g);}
 if(command=="max_plies"){max_plies(g,int(r.at("limit").integer(1,1000000)));return encode(g);}
 if(command=="legal"){auto list=array();if(!g.result.terminal)for(auto m:legal(g.position))list.arr.push_back(encode(m));return list;}
 if(command=="resign"){
  if(!g.result.terminal)g.result={true,"resign",other(g.position.turn),g.position.turn};return encode(g);
 }
 if(command=="forfeit"){
  auto loser=side(r.at("side").text());if(!g.result.terminal)g.result={true,"resign",other(loser),loser};return encode(g);
 }
 if(command=="history_view"){
  auto counts=object({});for(const auto&e:g.history)counts.obj[identity(e.position)]=number(e.count);return counts;
 }
 if(command=="validate_info"){
  auto info=r.at("info");ep::info(info);ep::require(info.at("id").number==r.at("id").integer(1,2147483647),"info ID");ep::require(info.at("multipv").number<=r.at("multipv").integer(1,5),"requested MultiPV");
  auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(r.obj.contains("budget_ms")?r.at("budget_ms").integer(1,100):50);
  std::unordered_map<std::string,int>counts;for(const auto&e:g.history){ep::require(std::chrono::steady_clock::now()<deadline,"info validation budget");counts.emplace(identity(e.position),e.count);}
  auto notations=array();auto p=g.position;bool terminal=g.result.terminal;
  for(const auto&j:info.at("pv").arr){
   ep::require(!terminal,"PV continues after terminal");ep::require(std::chrono::steady_clock::now()<deadline,"info validation budget");auto m=decode_move(j);auto moves=legal(p);ep::require(std::any_of(moves.begin(),moves.end(),[&](const Move&v){return same(v,m);}),"illegal PV action");
   notations.arr.push_back(text(format_move(p,m)));detail::apply_generated(p,m);terminal=++counts[identity(p)]>=3||legal(p).empty();
  }
  return object({{"info",info},{"notations",notations}});
 }
 throw ep::Error("BAD_COMMAND","unknown local rules API command");
}
}
extern "C" __declspec(dllexport) size_t clp_core_json(const char*request,char*reply,size_t capacity){
 std::string result;
 try{
  ep::require(request!=nullptr,"null request");size_t size=0;while(size<=ep::MAX_JSON&&request[size])++size;ep::require(size<=ep::MAX_JSON,"request limit");
  result=ep::dump(handle(ep::Parser(std::string(request,size)).parse()));ep::require(result.size()<=ep::MAX_JSON,"response payload limit");
 }catch(const std::exception&e){result=ep::dump(object({{"error",text(e.what())}}));}
 auto required=result.size()+1;if(reply&&capacity>=required)std::memcpy(reply,result.c_str(),required);return required;
}
