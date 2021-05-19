#ifndef OMOK_GAME_HPP
#define OMOK_GAME_HPP

#include <string>

class OmokPlayer;


//how the turn is determined:
//the leader has the first turn upon the creation of an Omok game, therefore is_leaders_turn_ is set to true
//when the winning move is placed, the turn is first switched and then the game is end, ensuring that
//the loser will have the head start in the next game.
class OmokGame{
public:
	OmokGame(int id, std::string room_name, int leader_id, std::string leader_name);
	~OmokGame();
	
	int get_id();
	std::string get_name();
	
	OmokPlayer *get_leader();
	void set_guest(int id, std::string name);
	OmokPlayer *get_guest();
	void remove_guest();
	bool get_is_leaders_turn();
	void set_is_leaders_turn(bool is_leaders_turn);
	const char *get_board();
	
	bool place_piece(int x, int y, bool is_leader);
	
	void set_remaining_time(int time_remaining);
	int get_time_remaining();
	int whose_turn();	//return player id
	void switch_turn();
	bool is_game_end();
	int who_won();
	void reset_board();	//makes the game ready to start again
private:
	int id_;
	std::string room_name_;
	OmokPlayer *leader_;
	OmokPlayer *guest_;
	char board_[225];
	int turn_end_time_;
	bool is_leaders_turn_;
	bool is_game_end_;
	int who_won_;
};
#endif