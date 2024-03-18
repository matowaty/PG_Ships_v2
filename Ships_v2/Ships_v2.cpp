#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

#define STAND_CHAR_LENGHT 20
#define FLEET_SIZE 10
#define NUMBER_OF_TYPES 4
#define BOARD_DEFAULT_X 10
#define BOARD_DEFAULT_Y 21
#define EMPTY_FIELD 1

enum Type {
	Car, Bat, Cru, Des
};

enum Direction_t {
	N,		//North
	S,		//South
	W,		//West
	E,		//East
};

struct Vector {
	int x{}, y{};
};

struct pozycja {
	int x{};
	int y{};
	Direction_t orient; //orientacja na plaszczyznie
};

struct Ship
{
	int x{};
	int y{};
	Direction_t orient = { N };
	int parts_alive = { 0 };
	bool placed = { 0 };
	int moves = { 0 };
	int shoot = { 0 };
};

struct Fleet {
	Ship ships[NUMBER_OF_TYPES][FLEET_SIZE]; //tablica typów statków wraz z informacjami na jego temat
	int max_amount_of_ships[NUMBER_OF_TYPES] = { 1,2,3,4 }; //maksymalna ilość statków na mapie
	int current_ship_amount[NUMBER_OF_TYPES] = { 0,0,0,0 }; //aktualna ilość danego typu na mapie
	static constexpr const int size_of_ships[NUMBER_OF_TYPES] = { 5,4,3,2 }; //długość statku
	char ship_type_char[4][4] = { "CAR","BAT","CRU","DES" };  //nazwa typu w formie napisu
	Vector init_position_1;
	Vector init_position_2;
};

struct Ship_info {
	int id_gracza;
	int ship_type;
	int ship_nr;
	int part_nr;
	bool hit = { 0 };
	bool empty = { true };
	bool Reef = { false };
	bool vision[2] = { false,false };
	bool spy = { false };
};

class Board {					//!!pamietać o & !!!! bo nie mam kopiowac!!!
	public:
	Ship_info** board;
	int x=0, y=0;  // maksymalny rozmiar tablicy
	//game_state.board.board[1][1]
	void init(int x,int y){  //inicjalizacja boarda o dowolnym rozmiarze
		this->x=x;
		this->y=y;
		this->board=new Ship_info*[y]();
		for(int i=0; i<y;i++){
			this->board[i]=new Ship_info[x]();
		}
	}
	~Board(){
		if(x>0 || y>0){
			for(int i=0; i<y;i++){
				delete[] board[i];
			} 
			delete[] board;
		}
	}

};

struct Player {
	Fleet fleet;
};

enum game_init_state {
	begin,
	board_change,
	game_start

};

struct Game_state {
	Board board;
	Player players[2];
	game_init_state init_state;
	int curr_player = 0;
	int Max_amount_of_reefs = { (BOARD_DEFAULT_X * BOARD_DEFAULT_Y) / 10 };
	bool ext_ship = false;
};

Type get_ship_type(char ship_type[4]) {
	if ((strcmp(ship_type, "CAR")) == 0)
		return Car;
	if ((strcmp(ship_type, "BAT")) == 0)
		return Bat;
	if ((strcmp(ship_type, "CRU")) == 0)
		return Cru;
	if ((strcmp(ship_type, "DES")) == 0)
		return Des;
	throw "BAD TYPE";
}

Direction_t convert_orient(char tmp_orient) {
	switch (tmp_orient)
	{
	case 'N':
		return N;
	case 'S':
		return S;
	case 'W':
		return W;
	case 'E':
		return E;
	default:
		throw;
		break;
	}
}

int switch_player(Game_state &game_state) {
	return (game_state.curr_player+1)%2;
}

char orient_to_char(Direction_t orient) {
	switch (orient)
	{
	case N:
		return 'N';
	case S:
		return 'S';
	case W:
		return 'W';
	case E:
		return 'E';
	default:
		throw;
		break;
	}
}

int status_check(char status[10]) {
	if (strcmp(status, "[playerA]") == 0)
		return 0;
	else if (strcmp(status, "[playerB]") == 0)
		return 1;
	else if (strcmp(status, "[state]") == 0)
		return 2;
	else if (strcmp(status, "-1") == 0)
		return -1;
	else
		return -1;
}

Vector transform(Direction_t orient) {
	switch (orient)
	{
	case N:
		return Vector{ 0,1 };
	case S:
		return Vector{ 0,-1 };
	case W:
		return Vector{ 1,0 };
	case E:
		return Vector{ -1,0 };
	default:
		throw;
		break;
	}
}

bool between(int value, int from, int to){
	
	return from<=value && value<=to;
}

bool is_ship_to_close(Game_state& game_state, Type type, int ship_num) {
	int x_check, y_check;
	bool to_close = false;
	Vector vector, left, right;
	vector = transform(game_state.players[game_state.curr_player].fleet.ships[type][ship_num].orient);
	x_check = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].x;
	y_check = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].y;

	auto& ship = game_state.board.board;
	auto& ship_info = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	auto& ship_lenght = game_state.players[game_state.curr_player].fleet.size_of_ships[type];

	if (vector.x != 0) { //poziomo
		left.x = 0;
		left.y = -1;
		right.x = 0;
		right.y = 1;
	}
	else {//pionowo

		left.x = -1;
		left.y = 0;
		right.x = 1;
		right.y = 0;
	}
				

	if ((x_check - vector.x < game_state.board.x && x_check-vector.x>=0)&&
		(y_check - vector.y < game_state.board.y && y_check-vector.y>=0)) {
		if (ship[y_check - vector.y][x_check - vector.x].empty == false && ship[y_check - vector.y][x_check - vector.x].Reef == false)
			to_close = true;
	}
	if ((x_check + vector.x * ship_lenght < game_state.board.x && x_check + vector.x * ship_lenght>=0) &&
		(y_check + vector.y * ship_lenght < game_state.board.y && y_check + vector.y * ship_lenght>=0)) {
		if (ship[y_check + vector.y * ship_lenght][x_check + vector.x * ship_lenght].empty == false && ship[y_check + vector.y * ship_lenght][x_check + vector.x * ship_lenght].Reef == false)
			to_close = true;
	}	

	for (int i = 0; i < ship_lenght; i++) {
		Vector curr_pos;
		curr_pos.x = x_check + vector.x * i;
		curr_pos.y = y_check + vector.y * i;

		if((curr_pos.x + right.x < game_state.board.x && curr_pos.x >= 0) &&
		    (curr_pos.y + right.y < game_state.board.y && curr_pos.y >= 0))
		{
			if (ship[curr_pos.y + right.y][curr_pos.x + right.x].empty == false && ship[curr_pos.y + right.y][curr_pos.x + right.x].Reef == false)
				to_close = true;
		}

		if ((curr_pos.x + left.x < game_state.board.x && curr_pos.x + left.x >= 0) &&
			(curr_pos.y + left.y < game_state.board.y && curr_pos.y + left.y >= 0))
		{
			if (ship[curr_pos.y + left.y][curr_pos.x + left.x].empty == false && ship[curr_pos.y + left.y][curr_pos.x + left.x].Reef == false)
				to_close = true;
		}

		if (ship[curr_pos.y][curr_pos.x].empty == false && ship[curr_pos.y][curr_pos.x].Reef == false)
			to_close = true;		

	}

	return to_close;
}

void is_ship_placed(Game_state &game_state, Type type, int ship_num) {
	auto& ship= game_state.players[game_state.curr_player].fleet.ships[type][ship_num];

	if (game_state.players[game_state.curr_player].fleet.ships[type][ship_num].placed) {
		cout << "INVALID OPERATION \"PLACE_SHIP " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[game_state.curr_player].fleet.ship_type_char[type] << "\": SHIP ALREADY PRESENT";
		exit(0);
	}

	if (game_state.players[game_state.curr_player].fleet.current_ship_amount[type] == game_state.players[game_state.curr_player].fleet.max_amount_of_ships[type]) {
		cout << "INVALID OPERATION \"PLACE_SHIP " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[game_state.curr_player].fleet.ship_type_char[type] << "\": ALL SHIPS OF THE CLASS ALREADY SET";
		exit(0);
	}
}

bool is_ship_on_reef(Game_state& game_state, Type type, int ship_num ) {
	Vector vector;
	int x, y;
	x = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].x;
	y = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].y;
	vector = transform(game_state.players[game_state.curr_player].fleet.ships[type][ship_num].orient);
	for (int i = 0; i < game_state.players[0].fleet.size_of_ships[type]; i++) {
		if (game_state.board.board[y + i * vector.y][x + i * vector.x].Reef == true)
			return true;
			
	}
	return false;
}

void board_update(Game_state &game_state, Vector vector, Type type, int ship_num) {
	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	for (int i = 0; i < game_state.players[game_state.curr_player].fleet.size_of_ships[type]; i++) {
		game_state.board.board[ship.y + (i * vector.y)][ship.x + (i * vector.x)].id_gracza = game_state.curr_player;   
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].part_nr = i;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_nr = ship_num;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_type = type;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].empty = false;

	}
}

void ship_placing(Game_state &game_state, int ship_num, Type type) {

	Vector vector = transform(game_state.players[game_state.curr_player].fleet.ships[type][ship_num].orient);
	int ship_end_x = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].x + game_state.players[game_state.curr_player].fleet.size_of_ships[type] * vector.x;
	int ship_end_y = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].y + game_state.players[game_state.curr_player].fleet.size_of_ships[type] * vector.y;
	auto& ship= game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	if (!((between(ship_end_y, 0, game_state.board.y - 1)||(between(ship_end_x,0,game_state.board.x-1))))){
		cout << "INVALID OPERATION \"PLACE_SHIP " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[game_state.curr_player].fleet.ship_type_char[type] << "\": NOT IN STARTING POSITION";
			exit(0);
	}	

	if (!((between(ship_end_y, 0, ((game_state.board.y - 1)/2)*(game_state.curr_player+1) )))) {
		cout << "INVALID OPERATION \"PLACE_SHIP " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[game_state.curr_player].fleet.ship_type_char[type] << "\": NOT IN STARTING POSITION";
		exit(0);
	}

	if (is_ship_on_reef(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"PLACE_SHIP " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << "\": PLACING SHIP ON REEF";
		exit(0);
	}

	if (is_ship_to_close(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"PLACE_SHIP " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << "\": PLACING SHIP TOO CLOSE TO OTHER SHIP";
		exit(0);

	}
	
	board_update(game_state, vector, type, ship_num);

}

void Place_ship_logic(int ship_num,Game_state &game_state, Type type, int x, int y, Direction_t orient) {
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].x = x;
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].y = y;
	is_ship_placed(game_state, type, ship_num);
	game_state.players[game_state.curr_player].fleet.current_ship_amount[type]++;	
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].orient = orient;
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].placed = 1;
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].parts_alive = game_state.players[game_state.curr_player].fleet.size_of_ships[type];
	ship_placing(game_state, ship_num, type);
	
}

void Place_ship(Game_state &game_state) {
	struct pozycja position;
	int ship_num = 0;
	char ship_type[4] = { 0 }, tmp_orient;



	scanf("%d %d %c %d %s", &position.y, &position.x, &tmp_orient, &ship_num, ship_type);

	position.orient = convert_orient(tmp_orient);

	if (position.y > game_state.board.y || position.x > game_state.board.x) {
		cout << "INVALID OPERATION \"PLACE_SHIP " << position.y << " " << position.x << " " << orient_to_char(position.orient) << " " << ship_num << " " << ship_type << "\": NOT IN STARTING POSITION";
		exit(0);
	}

	

	Type type = get_ship_type(ship_type);

	
	Place_ship_logic(ship_num, game_state, type, position.x, position.y,position.orient);
}

void invalid_player(int Q, int pl_turn) {
	if ((Q == 0 && pl_turn == 1) || (Q == 1 && pl_turn == 0)) {  //sprawdzenie czy gracz gra w swojej turze
		if (Q == 0) {
			cout << "INVALID OPERATION \"[playerA] \": THE OTHER PLAYER EXPECTED";
			exit(0);
		}
		if (Q == 1) {
			cout << "INVALID OPERATION \"[playerB] \": THE OTHER PLAYER EXPECTED";
			exit(0);
		}
	}
}

int parts_left(Game_state& game_state, int player) {
	int ctr=0;
	for (int i = 0; i < NUMBER_OF_TYPES; i++) {
		for (int j = 0; j < FLEET_SIZE; j++)
		{
			ctr += game_state.players[player].fleet.ships[i][j].parts_alive;
		}
	}
	return ctr;
}

void print_board(Game_state &game_state) {
	int y, x;
	y = game_state.board.y;
	x = game_state.board.x;

	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			if (game_state.board.board[i][j].empty == EMPTY_FIELD)
				cout << " ";
			else {
				if (game_state.board.board[i][j].Reef == true)
					cout << "#";
				else if (game_state.board.board[i][j].hit == true)
					cout << "x";
				else if (game_state.board.board[i][j].empty == false)
					cout << "+";
			}
		}
		cout << endl;
	}

	int A_cntr = 0, B_cntr = 0;

	A_cntr = parts_left(game_state, 0);
	B_cntr = parts_left(game_state, 1);

	cout << "PARTS REMAINING:: A : " << A_cntr << " B : " << B_cntr << endl;
}

void Set_fleet(Game_state &game_state) {
	int CAR;
	int BAT;
	int CRU;
	int DES;

	cin >> CAR;
	cin >> BAT;
	cin >> CRU;
	cin >> DES;

	auto& max_amount_of_ship = game_state.players[game_state.curr_player].fleet.max_amount_of_ships;
	max_amount_of_ship[Car] = CAR;
	max_amount_of_ship[Bat] = BAT;
	max_amount_of_ship[Cru] = CRU;
	max_amount_of_ship[Des] = DES;
}

void not_all_ships_placed(Game_state &game_state, int x, int y) {
	for (int i = 0; i < 4; i++) {
		if (game_state.players[0].fleet.current_ship_amount[i] < game_state.players[0].fleet.max_amount_of_ships[i]) {
			cout<< "INVALID OPERATION \"SHOOT " << y << " " << x << "\": NOT ALL SHIPS PLACED";
			exit(0);
		}
		if (game_state.players[1].fleet.current_ship_amount[i] < game_state.players[1].fleet.max_amount_of_ships[i]) {
			cout << "INVALID OPERATION \"SHOOT " << y << " " << x << "\": NOT ALL SHIPS PLACED";
			exit(0);
		}
	}
}

void Shoot(Game_state &game_state) {
	int enemy_player = switch_player(game_state);
	auto& ship = game_state.players[enemy_player].fleet;
	auto& board = game_state.board.board;
	int x, y;
	cin >> y;
	cin >> x;

	not_all_ships_placed(game_state,x,y);

	if (x > game_state.board.x || y > game_state.board.y) {
		cout << "INVALID OPERATION \"SHOOT " << y << " " << x << "\": FIELD DOES NOT EXIST";
		exit(0);
	}

	if (board[y][x].id_gracza == enemy_player) {
		if (!(board[y][x].hit)) {
			board[y][x].hit = true;
			if (ship.ships[board[y][x].ship_type][board[y][x].ship_nr].parts_alive != 0) {
				ship.ships[board[y][x].ship_type][board[y][x].ship_nr].parts_alive--;
			}
			
		}
		
		
	}
}

void check_win_condition(Game_state& game_state) {
	int A_parts, B_parts;

	A_parts = parts_left(game_state, 0);
	B_parts = parts_left(game_state, 1);

	if (A_parts == 0) {
		cout << "B won";
		exit(0);
	}
	if (B_parts == 0) {
		cout << "A won";
		exit(0);
	}
}

void Set_board_size(Game_state &game_state) {
	int x, y;
	cin >> y;
	cin >> x;

	game_state.board.init(x, y);
	game_state.init_state = game_start;
}

void Set_reef(Game_state& game_state) {
	int x, y;
	cin >> y;
	cin >> x;
	if (y > game_state.board.y || x > game_state.board.x) {
		cout<< "INVALID OPERATION \"REEF " << y << " " << x << "\": REEF IS NOT PLACED ON BOARD";
		exit(0);
	}
	if (game_state.board.board[y][x].empty = false) {
		cout << "INVALID OPERATION \"REEF " << y << " " << x << "\": REEF CANT BE PLACED ON SHIP";
		exit(0);
	}
	game_state.board.board[y][x].Reef = true;
	game_state.board.board[y][x].empty = false;

}

void Custom_board_update(Game_state& game_state, Vector vector, Type type, int ship_num,int *ship_status) {
	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	for (int i = 0; i < game_state.players[game_state.curr_player].fleet.size_of_ships[type]; i++) {
		game_state.board.board[ship.y + (i * vector.y)][ship.x + (i * vector.x)].id_gracza = game_state.curr_player;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].part_nr = i;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_nr = ship_num;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_type = type;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].empty = false;
		if (ship_status[i] == 0) {
			game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].hit = true;
		}

	}
}

void Custom_ship_placing(Game_state& game_state, int ship_num, Type type, int* ship_status) {

	Vector vector = transform(game_state.players[game_state.curr_player].fleet.ships[type][ship_num].orient);
	int ship_end_x = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].x + game_state.players[game_state.curr_player].fleet.size_of_ships[type] * vector.x;
	int ship_end_y = game_state.players[game_state.curr_player].fleet.ships[type][ship_num].y + game_state.players[game_state.curr_player].fleet.size_of_ships[type] * vector.y;
	char tmp_status[6];
	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];

	for (int i = 0; i < game_state.players[0].fleet.size_of_ships[type]; i++) {
		tmp_status[i] = (char)(ship_status[i]+48);
		tmp_status[i+1] = '\0';
	}

	if (!((between(ship_end_y, 0, game_state.board.y - 1) || (between(ship_end_x, 0, game_state.board.x - 1))))) {
		cout << "INVALID OPERATION \"SHIP " << game_state.curr_player <<ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[game_state.curr_player].fleet.ship_type_char[type]<< " " << tmp_status << "\": NOT IN STARTING POSITION";
		exit(0);
	}

	if (!((between(ship_end_y, 0, ((game_state.board.y - 1) / 2) * (game_state.curr_player + 1))))) {
		cout << "INVALID OPERATION \"SHIP " << (char)(game_state.curr_player+65) << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[game_state.curr_player].fleet.ship_type_char[type] << " " << tmp_status << "\": NOT IN STARTING POSITION";
		exit(0);
	}

	if (is_ship_to_close(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"SHIP " << (char)(game_state.curr_player + 65) << " " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << tmp_status << "\": PLACING SHIP TOO CLOSE TO OTHER SHIP";
		exit(0);
	}

	if (is_ship_on_reef(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"SHIP " << (char)(game_state.curr_player + 65) << " " << ship.y << " " << ship.x << " " << orient_to_char(ship.orient) << " " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << tmp_status << "\": PLACING SHIP ON REEF";
		exit(0);
	}

	Custom_board_update(game_state, vector, type, ship_num, ship_status);

}

void Place_custom_ship(Game_state& game_state, int x, int y, Type type, Direction_t orient, int ship_num, int* ship_status ) {
	int parts_alive=0;
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].x = x;
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].y = y;
	is_ship_placed(game_state, type, ship_num);
	game_state.players[game_state.curr_player].fleet.current_ship_amount[type]++;
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].orient = orient;
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].placed = 1;

	for (int i = 0; i < game_state.players[0].fleet.size_of_ships[type]; i++) {
		if (ship_status[i] == 1)
			parts_alive++;
	}
	game_state.players[game_state.curr_player].fleet.ships[type][ship_num].parts_alive = parts_alive;

	Custom_ship_placing(game_state, ship_num, type, ship_status);
}

void Set_custom_ship(Game_state &game_state) {
	char tmp_player, orient_c;
	char type_c[4], tmp_ship_status[6];
	int x, y, ship_num, player;
	int ship_status[5] = {0,0,0,0,0};
	Direction_t orient;
	Type type;

	cin >> tmp_player;
	cin >> y;
	cin >> x;
	cin >> orient_c;
	cin >> ship_num;
	cin >> type_c;
	cin >> tmp_ship_status;

	if (tmp_player == 'A')
		player = 0;
	else
		player = 1;

	orient = convert_orient(orient_c);
	type = get_ship_type(type_c);

	if (y > game_state.board.y || x > game_state.board.x) {
		cout << "INVALID OPERATION \"SHIP " << tmp_player << y << " " << x << " " << orient_c << " " << ship_num << " " << type << " " << tmp_ship_status << " " << "\": NOT IN STARTING POSITION";
		exit(0);
	}

	for (int i = 0; i < game_state.players[0].fleet.size_of_ships[type]; i++) {
		ship_status[i] = (int)tmp_ship_status[i]-48;
	}
	game_state.curr_player = player;
	Place_custom_ship(game_state, x, y, type, orient,ship_num, ship_status);
}

void Set_init_position(Game_state& game_state) {
	int x1, x2, y1, y2, player;
	char player_char;

	cin >> player_char;
	cin >> y1;
	cin >> x1;
	cin >> y2;
	cin >> x2;

	if (player_char == 'A')
		player = 0;
	else
		player = 1;

	game_state.players[player].fleet.init_position_1.x = x1;
	game_state.players[player].fleet.init_position_1.y = y1;
	game_state.players[player].fleet.init_position_2.x = x2;
	game_state.players[player].fleet.init_position_2.y = y2;

}

int calculate_new_x(Game_state& game_state, Direction_t orient, Type type, int ship_num) {
	Vector vector1, vector2;
	int x;
	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	vector1 = transform(ship.orient);
	vector2 = transform(orient);
	x = ship.x;
	x = x - vector1.x;
	x = x - (vector2.x * (game_state.players[game_state.curr_player].fleet.size_of_ships[type]-1));

	return x;

}

int calculate_new_y(Game_state& game_state, Direction_t orient, Type type, int ship_num) {
	Vector vector1, vector2;
	int y;
	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	vector1 = transform(ship.orient);
	vector2 = transform(orient);
	y = ship.y;
	y = y - vector1.y;
	y = y - (vector2.y * (game_state.players[game_state.curr_player].fleet.size_of_ships[type] - 1));

	return y;

}

void clear_ship_info(Game_state& game_state, int ship_num, Type type, Vector vector) {
	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	for (int i = 0; i < game_state.players[game_state.curr_player].fleet.size_of_ships[type]; i++) {
		game_state.board.board[ship.y + (i * vector.y)][ship.x + (i * vector.x)].id_gracza = NULL;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].part_nr = NULL;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_nr = NULL;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_type = NULL;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].empty = true;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].hit = false;

	}
	
}

void move_ship_side(Game_state& game_state, int ship_num, Type type, int ship_status[5], Direction_t orient, int x, int y, char err_dir) {

	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];

	Vector vector;

	vector = transform(orient);

	if (y > game_state.board.y || x > game_state.board.x || x < 0 || y < 0) {
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << err_dir << "\": SHIP WENT FROM BOARD";
		exit(0);
	}

	ship.x = x;
	ship.y = y;

	if (is_ship_to_close(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << err_dir << "\": PLACING SHIP TOO CLOSE TO OTHER SHIP";
		exit(0);
	}

	if (is_ship_on_reef(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << err_dir << "\": PLACING SHIP ON REEF";
		exit(0);
	}

	for (int i = 0; i < game_state.players[game_state.curr_player].fleet.size_of_ships[type]; i++) {
		game_state.board.board[ship.y + (i * vector.y)][ship.x + (i * vector.x)].id_gracza = game_state.curr_player;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].part_nr = i;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_nr = ship_num;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_type = type;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].empty = false;
		if (ship_status[i] == 0) {
			game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].hit = true;
		}

	}


}

void move_ship_forward(Game_state& game_state, int ship_num, Type type, int ship_status[5], Direction_t orient, int x, int y) {

	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];

	Vector vector;

	vector = transform(orient);

	if (y > game_state.board.y || x > game_state.board.x || x<0 || y<0) {
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " "<< "F" << "\": SHIP WENT FROM BOARD";
		exit(0);
	}

	ship.x = x;
	ship.y = y;

	if (is_ship_to_close(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << "F" << "\": PLACING SHIP TOO CLOSE TO OTHER SHIP";
		exit(0);
	}

	if (is_ship_on_reef(game_state, type, ship_num)) {
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << "F" << "\": PLACING SHIP ON REEF";
		exit(0);
	}

	for (int i = 0; i < game_state.players[game_state.curr_player].fleet.size_of_ships[type]; i++) {
		game_state.board.board[ship.y + (i * vector.y)][ship.x + (i * vector.x)].id_gracza = game_state.curr_player;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].part_nr = i;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_nr = ship_num;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].ship_type = type;
		game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].empty = false;
		if (ship_status[i] == 0) {
			game_state.board.board[ship.y + i * vector.y][ship.x + i * vector.x].hit = true;
		}

	}


}

void Move_ship(Game_state& game_state) {
	int ship_num, x, y, ship_status[5] = { 0,0,0,0,0 };
	Type type;
	Direction_t orient;
	char move_dir, tmp_type[4];
	Vector vector;
	

	cin >> ship_num;
	cin >> tmp_type;
	cin >> move_dir;

	type = get_ship_type(tmp_type);

	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];

	vector = transform(ship.orient);	

	x = ship.x;
	y = ship.y;

	orient = ship.orient;

	for (int i = 0; i < game_state.players[0].fleet.size_of_ships[type]; i++) {
		if (game_state.board.board[y + i * vector.y][x + i * vector.x].hit == 0)
			ship_status[i] = 1;
		else
			ship_status[i] = 0;		
	}

	if (ship.moves >= 3) {
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << move_dir << "\": SHIP MOVED ALREADY";
		exit(0);
	}

	if (ship_status[game_state.players[0].fleet.size_of_ships[type]-1] == 0) {	//error handler
		cout << "INVALID OPERATION \"MOVE " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << move_dir << "\": SHIP CANNOT MOVE";
		exit(0);
	}

	if (move_dir == 'F') {		//ship sails forward
		x = x - vector.x;
		y = y - vector.y;
		clear_ship_info(game_state, ship_num, type, vector);
		move_ship_forward(game_state, ship_num, type, ship_status, orient, x, y);
		ship.moves++;
	}

	if (move_dir == 'L') {
		clear_ship_info(game_state, ship_num, type, vector);
		switch (orient)
		{
		case N:
			orient = W;
			break;
		case S:
			orient = E;
			break;
		case W:
			orient = S;
			break;
		case E:
			orient = N;
			break;
		default:
			break;
		}
		x = calculate_new_x(game_state, orient, type, ship_num);
		y = calculate_new_y(game_state, orient, type, ship_num);
		ship.orient = orient;
		move_ship_side(game_state,ship_num,type,ship_status,orient,x,y,'L');
		ship.moves++;

	}
	if (move_dir == 'R') {
		clear_ship_info(game_state, ship_num, type, vector);
		switch (orient)
		{
		case N:
			orient = E;
			break;
		case S:
			orient = W;
			break;
		case W:
			orient = N;
			break;
		case E:
			orient = S;
			break;
		default:
			break;
		}
		x = calculate_new_x(game_state, orient, type, ship_num);
		y = calculate_new_y(game_state, orient, type, ship_num);
		ship.orient = orient;
		move_ship_side(game_state, ship_num, type, ship_status, orient, x, y, 'R');
		ship.moves++;
	}

}

void Reset_turn(Game_state& game_state) {
	for (int i = 1; i < 4; i++) {
		for (int j = 0; j < 10; j++) {
			game_state.players[game_state.curr_player].fleet.ships[i][j].moves = 0;
		}
	}
	for (int j = 0; j < 10; j++) {
		game_state.players[game_state.curr_player].fleet.ships[0][j].moves = 1;
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++) {
			game_state.players[game_state.curr_player].fleet.ships[i][j].shoot = 0;
		}
	}
}

void Advanced_shoot(Game_state& game_state) {
	int ship_num;
	char tmp_type[4];
	int x, y, canon_x, canon_y;
	Type type;
	Direction_t orient;
	Vector vector;

	cin >> ship_num;
	cin >> tmp_type;
	cin >> y;
	cin >> x;

	type = get_ship_type(tmp_type);
	auto& ship = game_state.players[game_state.curr_player].fleet.ships[type][ship_num];
	orient = ship.orient;
	vector = transform(orient);
	canon_x = ship.x + vector.x;
	canon_y = ship.y + vector.y;

	if (game_state.players[game_state.curr_player].fleet.ships[type][ship_num].shoot > game_state.players[game_state.curr_player].fleet.size_of_ships[type]-1) {
		cout << "INVALID OPERATION \"SHOOT " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << y << " " << x << "\": TOO MANY SHOOTS";
		exit(0);
	}

	if (game_state.board.board[canon_y][canon_x].hit == true) {
		cout << "INVALID OPERATION \"SHOOT " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << y << " " << x << "\": SHIP CANNOT SHOOT";
		exit(0);
	}

	if (type > 0) {
		double p1, p2;
		p1 = pow(canon_y - y, 2);
		p2 = pow(canon_x - x, 2);

		if (sqrt(p1 + p2) <= game_state.players[game_state.curr_player].fleet.size_of_ships[type]) {
			int enemy_player = switch_player(game_state);
			auto& ship = game_state.players[enemy_player].fleet;
			auto& board = game_state.board.board;

			if (board[y][x].id_gracza == enemy_player) {
				if (!(board[y][x].hit)) {
					board[y][x].hit = true;
					if (ship.ships[board[y][x].ship_type][board[y][x].ship_nr].parts_alive != 0) {
						ship.ships[board[y][x].ship_type][board[y][x].ship_nr].parts_alive--;
					}

				}


			}
			game_state.players[game_state.curr_player].fleet.ships[type][ship_num].shoot++;
		}
		else {
			cout << "INVALID OPERATION \"SHOOT " << ship_num << " " << game_state.players[0].fleet.ship_type_char[type] << " " << y << " " << x << "\": SHOOTING TOO FAR";
			exit(0);
		}
	}
	else {
		int enemy_player = switch_player(game_state);
		auto& ship = game_state.players[enemy_player].fleet;
		auto& board = game_state.board.board;

		if (board[y][x].id_gracza == enemy_player) {
			if (!(board[y][x].hit)) {
				board[y][x].hit = true;
				if (ship.ships[board[y][x].ship_type][board[y][x].ship_nr].parts_alive != 0) {
					ship.ships[board[y][x].ship_type][board[y][x].ship_nr].parts_alive--;
				}

			}


		}
		game_state.players[game_state.curr_player].fleet.ships[type][ship_num].shoot++;
	}

}

void vision_reset(Game_state& game_state) {
	auto& board = game_state.board.board;
	for (int i = 0; i < game_state.board.y; i++) {
		for (int j = 0; j < game_state.board.x; j++) {
			board[i][j].vision[game_state.curr_player] = false;
		}
	}
}

void vision_spy(Game_state& game_state) {

	auto& board = game_state.board.board;

	for (int i = 0; i < game_state.board.y; i++) {
		for (int j = 0; j < game_state.board.x; j++) {
			if (board[i][j].spy == true) {		//spy logic
				board[i][j].vision[game_state.curr_player] = true;					//pole drona


				if (i < game_state.board.y)											//pola obok drona
					board[i + 1][j].vision[game_state.curr_player] = true;
				if (i >= 1)
					board[i - 1][j].vision[game_state.curr_player] = true;
				if (j < game_state.board.x)
					board[i][j + 1].vision[game_state.curr_player] = true;
				if (j >= 1)
					board[i][j - 1].vision[game_state.curr_player] = true;


				if (i < game_state.board.y && j < game_state.board.x)				//przekątne
					board[i + 1][j + 1].vision[game_state.curr_player] = true;
				if (i >= 1 && j >= 1)
					board[i - 1][j - 1].vision[game_state.curr_player] = true;
				if (j < game_state.board.x && i >= 1)
					board[i - 1][j + 1].vision[game_state.curr_player] = true;
				if (j >= 1 && i < game_state.board.y)
					board[i + 1][j - 1].vision[game_state.curr_player] = true;
			}
		}
	}
}

void check_vision(Game_state& game_state) {

	int x, y;

	vision_reset(game_state);

	auto& game = game_state.players[game_state.curr_player].fleet;
	auto& board = game_state.board.board;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < game.max_amount_of_ships[i]; j++) {
			if (game.ships[i][j].placed == true) {
				x = game.ships[i][j].x;
				y = game.ships[i][j].y;
				if (board[y][x].hit == false) { //działający radar logika
					for (int k = 0; k < game_state.board.y; k++) {
						for (int l = 0; l < game_state.board.x; l++) {
							double p1, p2;
							p1 = pow(x - l, 2);
							p2 = pow(y - k, 2);

							if (sqrt(p1 + p2) <= game_state.players[game_state.curr_player].fleet.size_of_ships[i]) {
								board[k][l].vision[game_state.curr_player] = true;
							}
						}
					}
				}
				else {
					Vector vector;
					Direction_t orient;
					orient = game.ships[i][j].orient;
					vector = transform(orient);
					for (int k = 0; k < game_state.players[game_state.curr_player].fleet.size_of_ships[i]; k++) {
						board[y+k*vector.y][x+k*vector.x].vision[game_state.curr_player] = true;
					}
					if(y < game_state.board.y)
						board[y+1][x].vision[game_state.curr_player] = true;
					if(y>1)
						board[y-1][x].vision[game_state.curr_player] = true;
					if (x < game_state.board.x)
						board[y][x+1].vision[game_state.curr_player] = true;
					if (x > 1)
						board[y][x-1].vision[game_state.curr_player] = true;
					
				}
			}
		}
	}

	vision_spy(game_state);
}

void Personal_print(Game_state& game_state) {
	int y, x;
	y = game_state.board.y;
	x = game_state.board.x;

	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			if (game_state.board.board[i][j].vision[game_state.curr_player] == false)
				cout << "?";
			else {
				if (game_state.board.board[i][j].Reef == true)
					cout << "#";
				else if (game_state.board.board[i][j].hit == true)
					cout << "x";
				else if (game_state.board.board[i][j].empty == false)
					cout << "+";
				else if (game_state.board.board[i][j].empty == EMPTY_FIELD)
					cout << " ";
			}
		}
		cout << endl;
	}
}

void Send_spy(Game_state& game_state) {
	int x, y, ship_num;
	cin >> ship_num;
	cin >> y;
	cin >> x;

	game_state.board.board[y][x].spy = true;
	if (game_state.players[game_state.curr_player].fleet.ships[Car][ship_num].shoot >= game_state.players[game_state.curr_player].fleet.size_of_ships[Car]) {
		cout << "INVALID OPERATION \"SPY " << ship_num << " " << y << " " << x << "\": ALL PLANES SENT";
		exit(0);
	}

	int car_pos_x, car_pos_y;
	Vector vector;
	Direction_t orient;
	car_pos_x = game_state.players[game_state.curr_player].fleet.ships[Car][ship_num].x;
	car_pos_y = game_state.players[game_state.curr_player].fleet.ships[Car][ship_num].y;
	orient = game_state.players[game_state.curr_player].fleet.ships[Car][ship_num].orient;
	vector = transform(orient);

	if (game_state.board.board[car_pos_y+vector.y][car_pos_x+vector.x].hit==true) {
		cout << "INVALID OPERATION \"SPY " << ship_num << " " << y << " " << x << "\": CANNOT SEND PLANE";
		exit(0);
	}

	game_state.players[game_state.curr_player].fleet.ships[Car][ship_num].shoot++;
}

void advanced_print_board(Game_state& game_state) {
	int y, x;
	y = game_state.board.y;
	x = game_state.board.x;

	Vector line_counter;

	line_counter.x = 0;
	line_counter.y = 0;

	auto& board = game_state.board.board;

	cout << "  0123456789"<<endl;

	for (int i = 0; i < y; i++)
	{
		if (line_counter.y == 10) {
			line_counter.x++;
			line_counter.y = 0;
		}

		cout << line_counter.x << line_counter.y;

		for (int j = 0; j < x; j++){	

			if (board[i][j].empty == EMPTY_FIELD)
				cout << " ";
			else {
				if (board[i][j].Reef == true)
					cout << "#";
				else if (board[i][j].hit == true)
					cout << "x";
				else if (board[i][j].empty == false) {
					if (board[i][j].part_nr == 0)
						cout << "@";
					else if (board[i][j].part_nr == game_state.players[0].fleet.size_of_ships[board[i][j].ship_type] - 1)
						cout << "%";
					else if (board[i][j].part_nr == 1)
						cout << "!";
					else
						cout << "+";
				}
			}
		}
		cout << endl;
		line_counter.y++;
	}

	int A_cntr = 0, B_cntr = 0;

	A_cntr = parts_left(game_state, 0);
	B_cntr = parts_left(game_state, 1);

	cout << "PARTS REMAINING:: A : " << A_cntr << " B : " << B_cntr << endl;
}

void game() {

	Game_state game_state;

	game_state.init_state = board_change;

	

	int Q=0, pl_cmd = 0, pl_turn = 0;
	char status[STAND_CHAR_LENGHT], line[STAND_CHAR_LENGHT];

	while (Q != -1) {

		cin >> status;
		Q = status_check(status);

		invalid_player(Q, pl_turn);

		switch (Q)
		{
		case 0: {	//akcje gracza A
			game_state.curr_player = 0;
			Reset_turn(game_state);
			if (game_state.init_state == board_change) {
				game_state.board.init(BOARD_DEFAULT_X, BOARD_DEFAULT_Y);
				game_state.init_state = game_start;
			}				
			while (pl_cmd == 0) {
				cin >> line;
				if (strcmp(line, "[playerA]") == 0) {
					pl_cmd = 1;
					pl_turn = 1;
				}
				else if (strcmp(line, "PLACE_SHIP") == 0) {
					Place_ship(game_state);
				}
				else if (strcmp(line, "SHOOT") == 0) {
					if (game_state.ext_ship == false) {
						Shoot(game_state);
						check_win_condition(game_state);
					}
					else {
						Advanced_shoot(game_state);
						check_win_condition(game_state);
					}
				}
				else if (strcmp(line, "MOVE") == 0) {
					Move_ship(game_state);
					game_state.curr_player = 0;
				}
				else if (strcmp(line, "PRINT") == 0) {
					cin >> line;
					if (strcmp(line, "0") == 0) {
						check_vision(game_state);
						Personal_print(game_state);
					}					
				}
				else if (strcmp(line, "SPY") == 0) {
					Send_spy(game_state);
				}
			}


			pl_cmd = 0;
			break;
		}
		case 1: {	//akcje gracza B
			game_state.curr_player = 1;
			Reset_turn(game_state);
			if (game_state.init_state == board_change) {
				game_state.board.init(BOARD_DEFAULT_X, BOARD_DEFAULT_Y);
				game_state.init_state = game_start;
			}
			while (pl_cmd == 0) {
				cin >> line;
				if (strcmp(line, "[playerB]") == 0) {
					pl_cmd = 1;
					pl_turn = 0;
				}
				else if (strcmp(line, "PLACE_SHIP") == 0) {
					Place_ship(game_state);
				}
				else if (strcmp(line, "SHOOT") == 0) {
					if (game_state.ext_ship == false) {
						Shoot(game_state);
						check_win_condition(game_state);
					}
					else {
						Advanced_shoot(game_state);
						check_win_condition(game_state);
					}
					
				}
				else if (strcmp(line, "MOVE") == 0) {
					Move_ship(game_state);
					game_state.curr_player = 1;
				}
				else if (strcmp(line, "PRINT") == 0) {
					cin >> line;
					if (strcmp(line, "0") == 0) {
						check_vision(game_state);
						Personal_print(game_state);
					}

				}
				else if (strcmp(line, "SPY") == 0) {
					Send_spy(game_state);
				}
			}


			pl_cmd = 0;
			break;
		}
		case 2: {	//akcje stanu gry
			while (pl_cmd == 0) {
				cin >> line;
				if (strcmp(line, "[state]") == 0) {
					pl_cmd = 1;
				}
				else if (strcmp(line, "BOARD_SIZE") == 0) {
					if (game_state.init_state == board_change) {
						Set_board_size(game_state);
					}
				}
				else if(game_state.init_state == board_change) {
					game_state.board.init(BOARD_DEFAULT_X, BOARD_DEFAULT_Y);
					game_state.init_state = game_start;
					
					if (strcmp(line, "PRINT") == 0) {
					cin >> line;
						if (strcmp(line, "0") == 0)
							print_board(game_state);
						else if (strcmp(line, "1") == 0)
							advanced_print_board(game_state);
					}
					else if (strcmp(line, "REEF") == 0) {
						Set_reef(game_state);
					}
					else if (strcmp(line, "SET_FLEET") == 0) {
						cin >> line;
						if (strcmp(line, "A") == 0) {
							game_state.curr_player = 0;
							Set_fleet(game_state);
						}
						else if (strcmp(line, "B") == 0) {
							game_state.curr_player = 1;
							Set_fleet(game_state);
						}
						game_state.curr_player = 0;
					}
					else if (strcmp(line, "SHIP") == 0) {
						Set_custom_ship(game_state);
					}
					else if (strcmp(line, "EXTENDED_SHIPS") == 0) {
						game_state.ext_ship = true;
					}
					else if (strcmp(line, "INIT_POSITION") == 0) {
						Set_init_position(game_state);
					}
				}
				else if (strcmp(line, "PRINT") == 0) {
					cin >> line;
					if (strcmp(line, "0") == 0)
						print_board(game_state);
					else if (strcmp(line, "1") == 0)
						advanced_print_board(game_state);
				}
				else if (strcmp(line, "REEF") == 0) {
					Set_reef(game_state);
				}
				else if (strcmp(line, "SET_FLEET") == 0) {
					cin >> line;
					if (strcmp(line, "A") == 0) {
						game_state.curr_player = 0;
						Set_fleet(game_state);
					}
					else if (strcmp(line, "B") == 0) {
						game_state.curr_player = 1;
						Set_fleet(game_state);
					}
					game_state.curr_player = 0;
				}
				else if (strcmp(line, "SHIP") == 0) {
					Set_custom_ship(game_state);
				}
				else if (strcmp(line, "EXTENDED_SHIPS") == 0) {
					game_state.ext_ship = true;
				}
				else if (strcmp(line, "INIT_POSITION") == 0) {
					Set_init_position(game_state);
				}
			}
			pl_cmd = 0;
			break;
		}
		default:
			break;
		}
	}

}

int main()
{
	game();  
}
