#include "core_search.h"
#include <algorithm>
#include <sstream>
namespace ryuugin {
using ep::J;
namespace {
const std::array<std::string,8>names{"VUONG","TUONG","SI","KI","XA","LONG","PHUONG","BINH"};
std::string side_name(Side s){return s==Side::Cam?"CAM":"XANH";}
Side side_of(const std::string&s){ep::require(ep::side(s),"side");return s=="CAM"?Side::Cam:Side::Xanh;}
PieceType type_of(const std::string&s){auto i=std::find(names.begin(),names.end(),s);ep::require(i!=names.end(),"piece type");return PieceType(i-names.begin());}
bool rotate(PieceType t){return t!=PieceType::Vuong&&t!=PieceType::Long&&t!=PieceType::Phuong;}
int idx(Side s){return static_cast<int>(s);}
J js(const std::string&s){J v;v.type=J::String;v.str=s;return v;}
J ji(int n){J v;v.type=J::Int;v.number=n;return v;}
J jb(bool n){J v;v.type=J::Bool;v.number=n;return v;}
J array(){J v;v.type=J::Array;return v;}
J object(std::initializer_list<std::pair<const std::string,J>> fields){J v;v.type=J::Object;v.obj=fields;return v;}
J sq(std::optional<Square>s){if(!s)return J{};auto a=array();a.arr={ji(s->row),ji(s->col)};return a;}
std::optional<Square>sq_of(const J&v){if(v.type==J::Null)return {};ep::square(v);return Square{int(v.arr[0].number),int(v.arr[1].number)};}
struct Vector {int r,c;bool sliding;};
std::vector<Vector> vectors(PieceType t){
 switch(t){
 case PieceType::Vuong:return {{-1,-1,false},{-1,0,false},{-1,1,false},{0,-1,false},{0,1,false},{1,-1,false},{1,0,false},{1,1,false}};
 case PieceType::Tuong:return {{-1,0,false},{-1,-1,false},{-1,1,false},{0,-1,false},{0,1,false},{1,0,false}};
 case PieceType::Si:return {{-1,0,false},{-1,-1,false},{-1,1,false},{1,-1,false},{1,1,false}};
 case PieceType::Ki:return {{-2,-1,false},{-2,1,false},{2,0,false},{2,-2,false},{2,2,false}};
 case PieceType::Xa:return {{-1,0,true}};
 case PieceType::Long:return {{-1,0,true},{1,0,true},{0,-1,true},{0,1,true}};
 case PieceType::Phuong:return {{-1,-1,true},{-1,1,true},{1,-1,true},{1,1,true}};
 case PieceType::Binh:return {{-1,0,false}};
 }throw ep::Error("INVALID_POSITION","type enum");
}
bool bounds(int r,int c){return r>=0&&r<9&&c>=0&&c<9;}
std::vector<Square> targets(const Position&p,Square src){auto&st=p.board[src.index()];std::vector<Square>out;if(st.empty())return out;auto top=st.back();for(auto v:vectors(top.type)){
 int r=v.r,c=v.c;for(int n=0;n<(rotate(top.type)?int(top.facing):0);++n){int old=r;r=c;c=-old;}
 int row=src.row+r,col=src.col+c;while(bounds(row,col)){Square dst{row,col};out.push_back(dst);if(!v.sliding||!p.board[dst.index()].empty())break;row+=r;col+=c;}
 }return out;}
bool king(const Stack&st){return std::any_of(st.begin(),st.end(),[](const Piece&p){return p.type==PieceType::Vuong;});}
bool same(const Move&a,const Move&b){return a.kind==b.kind&&a.src==b.src&&a.dst==b.dst&&a.type==b.type&&a.facing==b.facing&&a.rotation==b.rotation;}
void transition(Position&p,const Move&m){ // private; callers prove membership in generated legal actions
 auto player=p.turn;int side=idx(player);
 if(m.kind==Kind::Drop){auto&stock=std::find(p.reserve[side].begin(),p.reserve[side].end(),m.type)!=p.reserve[side].end()?p.reserve[side]:p.prisoners[side];auto it=std::find(stock.begin(),stock.end(),m.type);ep::require(it!=stock.end(),"drop stock");stock.erase(it);p.board[m.dst->index()].push_back({m.type,player,*m.facing});}
 else {auto&source=p.board[m.src->index()];if(m.kind==Kind::Rotate){source.back().facing=Facing((int(source.back().facing)+m.rotation)%4);}else{auto&dst=p.board[m.dst->index()];if(!dst.empty()&&dst.back().owner!=player){for(auto q:dst)p.prisoners[side].push_back(q.type);dst.clear();}Stack moving;if(m.kind==Kind::Single){moving.push_back(source.back());source.pop_back();}else{moving=std::move(source);source.clear();}moving.back().facing=Facing((int(moving.back().facing)+m.rotation)%4);dst.insert(dst.end(),moving.begin(),moving.end());}}
 p.turn=other(player);
}
void check_result(const GameState&g){auto moves=legal(g.position);bool no=moves.empty();bool ck=in_check(g.position,g.position.turn);if(!g.result.terminal)ep::require(!no,"ongoing no legal moves");else if(g.result.reason=="checkmate"||g.result.reason=="pat")ep::require(no&&ck==(g.result.reason=="checkmate")&&g.result.loser==g.position.turn&&g.result.winner==other(g.position.turn),"terminal claim disagrees with rules");}
}
int Square::index()const{ep::require(bounds(row,col),"square bounds");return row*9+col;}
Side other(Side s){ep::require(s==Side::Cam||s==Side::Xanh,"side enum");return s==Side::Cam?Side::Xanh:Side::Cam;}
J encode(const Position&p){auto b=array();for(auto&st:p.board){auto cell=array();for(auto&q:st){ep::require(int(q.type)>=0&&int(q.type)<8&&int(q.owner)>=0&&int(q.owner)<2,"piece enum");cell.arr.push_back(object({{"type",js(names[int(q.type)])},{"owner",js(side_name(q.owner))},{"facing",ji(int(q.facing))}}));}b.arr.push_back(cell);}auto hands=[&](const Hands&h){J result=object({{"CAM",array()},{"XANH",array()}});for(int side=0;side<2;++side)for(auto t:h[side]){ep::require(int(t)>=0&&int(t)<8,"hand enum");result.obj[side==0?"CAM":"XANH"].arr.push_back(js(names[int(t)]));}return result;};ep::require(p.turn==Side::Cam||p.turn==Side::Xanh,"side enum");return object({{"board",b},{"to_move",js(side_name(p.turn))},{"reserve",hands(p.reserve)},{"prisoners",hands(p.prisoners)}});}
void validate(const Position&p){
 ep::require(p.turn==Side::Cam||p.turn==Side::Xanh,"side enum");
 int total=0;std::array<int,2>kings{};
 for(const auto&st:p.board){
  ep::require(st.size()<=38,"stack38");
  for(const auto&q:st){
   ep::require(int(q.type)>=0&&int(q.type)<8&&int(q.owner)>=0&&int(q.owner)<2&&int(q.facing)>=0&&int(q.facing)<4,"piece enum");
   ep::require(q.owner==st.front().owner,"mixed team");++total;
   if(q.type==PieceType::Vuong){ep::require(st.size()==1,"king in team");++kings[int(q.owner)];}
  }
 }
 for(int side=0;side<2;++side){
  std::array<bool,8>seen{};
  for(auto t:p.reserve[side]){ep::require(t==PieceType::Long||t==PieceType::Phuong,"reserve type");ep::require(!seen[int(t)],"reserve unique");seen[int(t)]=true;++total;}
  for(auto t:p.prisoners[side]){ep::require(int(t)>0&&int(t)<8,"prisoner type");++total;}
 }
 ep::require(kings[0]==1&&kings[1]==1&&total<=40,"king/40 bound");
}
namespace detail {
void apply_generated(Position&p,const Move&m){transition(p,m);}
std::vector<Geometry>geometry(PieceType t,Facing f){
 ep::require(int(t)>=0&&int(t)<8&&int(f)>=0&&int(f)<4,"geometry enum");std::vector<Geometry>out;
 for(auto v:vectors(t)){int r=v.r,c=v.c;for(int n=0;n<(rotate(t)?int(f):0);++n){int old=r;r=c;c=-old;}out.push_back({r,c,v.sliding});}return out;
}
std::vector<Move>record_moves(const Position&p,bool legacy){
 auto base=legal(p);if(!legacy)return base;auto out=base;std::vector<Square>rotated;
 for(auto m:base){
  if(!rotate(m.type))continue;
  if((m.kind==Kind::Single||m.kind==Kind::Team)&&m.rotation==0){m.rotation=2;out.push_back(m);}
  if(m.kind==Kind::Rotate&&std::find(rotated.begin(),rotated.end(),*m.src)==rotated.end()){rotated.push_back(*m.src);m.rotation=2;out.push_back(m);}
 }return out;
}
Undo make_record(GameState&g,const Move&m,bool legacy){
 if(m.rotation!=2)return make(g,m);
 ep::require(legacy&&rotate(m.type)&&m.kind!=Kind::Drop,"archival180 context required");ep::state(encode(g));ep::require(!g.result.terminal,"game terminal");
 auto normalized=m;normalized.rotation=m.kind==Kind::Rotate?1:0;ep::move(encode(normalized));
 auto moves=record_moves(g.position,true);ep::require(std::any_of(moves.begin(),moves.end(),[&](auto&v){return same(v,m);}),"illegal archival move");
 auto child=g;transition(child.position,m);++child.ply;
 auto key=identity(child.position);auto it=std::find_if(child.history.begin(),child.history.end(),[&](auto&e){return identity(e.position)==key;});
 if(it==child.history.end())child.history.push_back({child.position,1});else ++it->count;
 adjudicate(child);ep::state(encode(child));Undo undo{g,ep::dump(encode(child))};g=std::move(child);return undo;
}
std::string format_record(const Position&p,const Move&m){
 if(m.rotation!=2)return format_move(p,m);
 ep::require(rotate(m.type)&&m.kind!=Kind::Drop&&m.src.has_value(),"archival formatter fields");
 auto adjusted=p;auto live=m;live.rotation=1;auto&st=adjusted.board[m.src->index()];ep::require(!st.empty(),"record source");st.back().facing=Facing((int(st.back().facing)+1)%4);return format_move(adjusted,live);
}
}
Position decode_position(const J&v){ep::position(v);Position p;p.turn=side_of(v.at("to_move").str);for(size_t i=0;i<81;++i)for(auto&q:v.at("board").arr[i].arr)p.board[i].push_back({type_of(q.at("type").str),side_of(q.at("owner").str),Facing(q.at("facing").number)});for(int n=0;n<2;++n)for(auto hand:{"reserve","prisoners"})for(auto&t:v.at(hand).at(n==0?"CAM":"XANH").arr)(std::string(hand)=="reserve"?p.reserve[n]:p.prisoners[n]).push_back(type_of(t.str));return p;}
std::string identity(const Position&p){return ep::identity(encode(p));}
J encode(const Move&m){auto k=m.kind==Kind::Single?"single":m.kind==Kind::Team?"team":m.kind==Kind::Rotate?"rotate":"drop";ep::require(int(m.kind)>=0&&int(m.kind)<=3&&int(m.type)>=0&&int(m.type)<8,"move enum");return object({{"schema",js("clp.move/1")},{"kind",js(k)},{"src",sq(m.src)},{"dst",sq(m.dst)},{"ptype",js(names[int(m.type)])},{"facing",m.facing?ji(int(*m.facing)):J{}},{"rotate_after",ji(m.rotation)},{"notation",m.notation}});}
Move decode_move(const J&v){ep::move(v);auto k=v.at("kind").str;Move m{ k=="single"?Kind::Single:k=="team"?Kind::Team:k=="rotate"?Kind::Rotate:Kind::Drop,sq_of(v.at("src")),sq_of(v.at("dst")),type_of(v.at("ptype").str),{},int(v.at("rotate_after").number),v.at("notation")};if(v.at("facing").type!=J::Null)m.facing=Facing(v.at("facing").number);return m;}
J encode(const GameState&g){auto entries=array();for(auto&e:g.history)entries.arr.push_back(object({{"position",encode(e.position)},{"count",ji(e.count)}}));J res=g.result.terminal?object({{"status",js("terminal")},{"reason",js(g.result.reason)},{"winner",g.result.winner?js(side_name(*g.result.winner)):J{}},{"loser",g.result.loser?js(side_name(*g.result.loser)):J{}}}):object({{"status",js("ongoing")}});return object({{"schema",js("clp.state/1")},{"position",encode(g.position)},{"ply",ji(g.ply)},{"repetition",object({{"complete",jb(true)},{"entries",entries}})},{"result",res},{"notation",g.notation}});}
GameState decode_state(const J&v,bool semantic){ep::state(v);GameState g;g.position=decode_position(v.at("position"));g.ply=int(v.at("ply").number);g.notation=v.at("notation");for(auto&e:v.at("repetition").at("entries").arr)g.history.push_back({decode_position(e.at("position")),int(e.at("count").number)});auto&r=v.at("result");g.result.terminal=r.at("status").str=="terminal";if(g.result.terminal){g.result.reason=r.at("reason").str;if(r.at("winner").type!=J::Null)g.result.winner=side_of(r.at("winner").str);if(r.at("loser").type!=J::Null)g.result.loser=side_of(r.at("loser").str);}if(semantic)check_result(g);return g;}
bool attacked(const Position&p,Square target,Side by){validate(p);ep::require(by==Side::Cam||by==Side::Xanh,"attack side");target.index();for(int i=0;i<81;++i)if(!p.board[i].empty()&&p.board[i].back().owner==by){auto list=targets(p,{i/9,i%9});if(std::find(list.begin(),list.end(),target)!=list.end())return true;}return false;}
bool in_check(const Position&p,Side side){for(int i=0;i<81;++i)if(!p.board[i].empty()&&p.board[i].back().type==PieceType::Vuong&&p.board[i].back().owner==side)return attacked(p,{i/9,i%9},other(side));throw ep::Error("INVALID_POSITION","missing king");}
std::vector<Move>pseudo(const Position&p){validate(p);std::vector<Move>out;int side=idx(p.turn);auto add=[&](Kind k,std::optional<Square>src,std::optional<Square>dst,PieceType t,std::optional<Facing>f,int rot){out.push_back({k,src,dst,t,f,rot,{}});};
 bool reserved=!p.reserve[side].empty();auto stock=reserved?p.reserve[side]:p.prisoners[side];std::sort(stock.begin(),stock.end());stock.erase(std::unique(stock.begin(),stock.end()),stock.end());for(auto t:stock)for(int dst=0;dst<81;++dst){if(reserved&&dst/9!=(p.turn==Side::Cam?1:7))continue;auto&st=p.board[dst];if(!st.empty()&&(st.back().owner!=p.turn||king(st)||st.size()>=38))continue;std::vector<int>facings=rotate(t)?std::vector<int>{0,1,2,3}:std::vector<int>{p.turn==Side::Cam?2:0};for(int f:facings)add(Kind::Drop,{},Square{dst/9,dst%9},t,Facing(f),0);}if(reserved)return out;
 for(int i=0;i<81;++i){auto&st=p.board[i];if(st.empty()||st.back().owner!=p.turn)continue;auto top=st.back();Square src{i/9,i%9};auto rots=rotate(top.type)?std::vector<int>{0,1,3}:std::vector<int>{0};for(auto dst:targets(p,src)){auto&target=p.board[dst.index()];bool own=!target.empty()&&target.back().owner==p.turn;if(!target.empty()&&!own&&king(target))continue;bool single=!own||(st.size()==1&&!king(st)&&!king(target)&&target.size()<38);for(int r:rots){if(single)add(Kind::Single,src,dst,top.type,{},r);if(st.size()>1&&!own&&!king(st))add(Kind::Team,src,dst,top.type,{},r);}}if(rotate(top.type))for(int r:{1,3})add(Kind::Rotate,src,{},top.type,{},r);}
 return out;}
std::vector<Move>legal(const Position&p){std::vector<Move>out;for(auto&m:pseudo(p)){auto child=p;transition(child,m);if(!in_check(child,p.turn))out.push_back(m);}return out;}
void adjudicate(GameState&g){auto key=identity(g.position);auto it=std::find_if(g.history.begin(),g.history.end(),[&](auto&e){return identity(e.position)==key;});ep::require(it!=g.history.end(),"current history missing");if(it->count>=3){g.result={true,"repetition3",g.position.turn,other(g.position.turn)};return;}if(legal(g.position).empty())g.result={true,in_check(g.position,g.position.turn)?"checkmate":"pat",other(g.position.turn),g.position.turn};}
Undo make(GameState&g,const Move&m){ep::move(encode(m));ep::state(encode(g));ep::require(!g.result.terminal,"game terminal");auto moves=legal(g.position);ep::require(std::any_of(moves.begin(),moves.end(),[&](auto&x){return same(x,m);}),"illegal move");auto child=g;transition(child.position,m);++child.ply;auto key=identity(child.position);auto it=std::find_if(child.history.begin(),child.history.end(),[&](auto&e){return identity(e.position)==key;});if(it==child.history.end())child.history.push_back({child.position,1});else ++it->count;adjudicate(child);ep::state(encode(child));Undo undo{g,ep::dump(encode(child))};g=std::move(child);return undo;}
void unmake(GameState&g,const Undo&u){ep::require(ep::dump(encode(g))==u.after,"stale undo");g=u.before;}
void max_plies(GameState&g,int limit){ep::require(limit>0,"max plies positive");if(!g.result.terminal&&g.ply>=limit)g.result={true,"max_plies",{}, {}};}
GameState initial(){GameState g;std::array<PieceType,9>back{PieceType::Xa,PieceType::Ki,PieceType::Si,PieceType::Tuong,PieceType::Vuong,PieceType::Tuong,PieceType::Si,PieceType::Ki,PieceType::Xa};for(int s=0;s<2;++s){Side side=Side(s);Facing f=s==0?Facing::Down:Facing::Up;for(int c=0;c<9;++c){g.position.board[(s==0?0:8)*9+c].push_back({back[c],side,f});g.position.board[(s==0?2:6)*9+c].push_back({PieceType::Binh,side,f});}g.position.reserve[s]={PieceType::Long,PieceType::Phuong};}g.history.push_back({g.position,1});return g;}
std::string format_move(const Position&p,const Move&m){validate(p);ep::move(encode(m));if(m.src){auto&source=p.board[m.src->index()];ep::require(!source.empty()&&source.back().owner==p.turn&&source.back().type==m.type,"notation source/type mismatch");}auto square=[](Square s){s.index();return std::string(1,char('a'+s.row))+char('9'-s.col);};std::string codes="VTSKXLPB";std::string text(1,codes[int(m.type)]);if(m.kind==Kind::Team)text='['+text+']';if(m.kind==Kind::Drop)text+='*';text+=square(m.dst?*m.dst:*m.src);if(rotate(m.type)){int f=m.kind==Kind::Drop?int(*m.facing):int(p.board[m.src->index()].back().facing);text+="^>v<"[(f+m.rotation)%4];}if(m.kind!=Kind::Drop)text+='.'+square(*m.src);return text;}
Move parse_move(const Position&p,std::string text){
 ep::require(ep::utf8(text)&&text.size()<=256,"notation limit/UTF8");
 for(auto pair:{std::pair<std::string,std::string>{"˄","^"},{"˅","v"}}){size_t i;while((i=text.find(pair.first))!=std::string::npos)text.replace(i,pair.first.size(),pair.second);}
 bool mate=!text.empty()&&text.back()=='#';if(mate)text.pop_back();std::vector<Move>matches;
 for(auto&m:legal(p)){auto full=format_move(p,m);auto dot=full.find('.');auto compact=dot==std::string::npos?full:full.substr(0,dot);if(text==full||text==compact){if(mate){auto child=p;transition(child,m);if(!in_check(child,child.turn)||!legal(child).empty())continue;}matches.push_back(m);}}
 ep::require(matches.size()==1,"notation illegal/ambiguous");auto m=matches.front();m.notation=js(text+(mate?"#":""));return m;
}
}
