#include "omok_game.hpp"
#include "omok_player.hpp"

#include <cstring>
#include <ctime>

OmokGame::OmokGame(int id, std::string room_name, int leader_id, std::string leader_name):
	id_(id),
	room_name_(room_name),
	leader_(new OmokPlayer(leader_id, leader_name)),
	guest_(0),
	board_(),
	turn_end_time_(0),
	is_leaders_turn_(true),
	is_game_end_(false),
	who_won_(0)
{
}

OmokGame::~OmokGame(){
	//remove the leader from the omok game
	//if theres another player in the omok game, notify him/her that the room is closed
	
	delete leader_;
	if(guest_){
		delete guest_;
	}
}

int OmokGame::get_id(){
	return id_;
}

std::string OmokGame::get_name(){
	return room_name_;
}

OmokPlayer *OmokGame::get_leader(){
	return leader_;
}

void OmokGame::set_guest(int id, std::string name){
	if(guest_){
		delete guest_;
	}
	
	guest_ = new OmokPlayer(id, name);
}

OmokPlayer *OmokGame::get_guest(){
	return guest_;
}

void OmokGame::remove_guest(){
	delete guest_;
	guest_ = 0;
}

bool OmokGame::get_is_leaders_turn(){
	 return is_leaders_turn_;
}

void OmokGame::set_is_leaders_turn(bool is_leaders_turn){
	is_leaders_turn_ = is_leaders_turn;
}

const char *OmokGame::get_board(){
	return board_;
}

bool OmokGame::place_piece(int x, int y, bool is_leader){
	//if a piece already takes that spot
	if(board_[y * 15 + x]){
		return false;
	}
	
	char sides_open[8] = {0}, num_aligned[8] = {0};
	char num_3 = 0, num_open_3 = 0;
	
	enum PlayerType{LEADER = 1, GUEST};
	
	char matching_piece;
	if(is_leader){
		matching_piece = LEADER;
	}
	else{
		matching_piece = GUEST;
	}
	//check in this configuration:
	/*
		012
		7x3
		654
	*/
	
	int i, j;
	
	//side 0
	for(i = x - 1, j = y - 1; i >= 0 && j >= 0; --i, --j){
		if(board_[j * 15 + i] == matching_piece){
			num_aligned[0]++;
		}
		else{
			if(!board_[j * 15 + i]){
				sides_open[0] = 1;
			}
			break;
		}
	}
	//side 1
	for(j = y - 1; j >= 0; --j){
		if(board_[j * 15 + x] == matching_piece){
			num_aligned[1]++;
		}
		else{
			if(!board_[j * 15 + x]){
				sides_open[1] = 1;
			}
			break;
		}
	}
	//2
	for(i = x + 1, j = y - 1; i < 15 && j >= 0; ++i, --j){
		if(board_[j * 15 + i] == matching_piece){
			num_aligned[2]++;
		}
		else{
			if(!board_[j * 15 + i]){
				sides_open[2] = 1;
			}
			break;
		}
	}
	//3
	for(i = x + 1; i < 15; ++i){
		if(board_[y * 15 + i] == matching_piece){
			num_aligned[3]++;
		}
		else{
			if(!board_[y * 15 + i]){
				sides_open[3] = 1;
			}
			break;
		}
	}
	//4
	for(i = x + 1, j = y + 1; i < 15 && j < 15; ++i, ++j){
		if(board_[j * 15 + i] == matching_piece){
			num_aligned[4]++;
		}
		else{
			if(!board_[j * 15 + i]){
				sides_open[4] = 1;
			}
			break;
		}
	}
	//5
	for(j = y + 1; j < 15; ++j){
		if(board_[j * 15 + x] == matching_piece){
			num_aligned[5]++;
		}
		else{
			if(!board_[j * 15 + x]){
				sides_open[5] = 1;
			}
			break;
		}
	}
	//6
	for(i = x - 1, j = y + 1; i >= 0 && j < 15; --i, ++j){
		if(board_[j * 15 + i] == matching_piece){
			num_aligned[6]++;
		}
		else{
			if(!board_[j * 15 + i]){
				sides_open[6] = 1;
			}
			break;
		}
	}
	//7
	for(i = x - 1; i >= 0; --i){
		if(board_[y * 15 + i] == matching_piece){
			num_aligned[7]++;
		}
		else{
			if(!board_[y * 15 + i]){
				sides_open[7] = 1;
			}
			break;
		}
	}
	
	//check for open 3's and closed 3's
	for(int line = 0; line < 4; ++line){
		bool is_3 = num_aligned[line] + num_aligned[line + 4] + 1 == 3;
		bool is_open_3 = sides_open[line] && sides_open[line + 4];
		
		if(is_3){
			num_3++;
		}
		if(is_open_3){
			num_open_3++;
		}
	}
	
	//if the move is a winning move, allow it, regardless of
	//if it makes 2 lines of 3
	//check if the player has won
	for(int line = 0; line < 4; ++line){
		if(num_aligned[line] + num_aligned[line + 4] + 1 == 5){
			is_game_end_ = true;
			
			if(is_leader){
				who_won_ = leader_->get_id();
			}
			else{
				who_won_ = guest_->get_id();
			}
			
			//place the piece
			board_[y * 15 + x] = matching_piece;
			
			return true;
		}
	}
	
	//if there is at least one open 3 and 2 open/closed 3's,
	//the move can't be placed
	if(num_3 >= 2){
		return false;
	}
	
	//place the piece
	board_[y * 15 + x] = matching_piece;
	
	
	return true;
}

void OmokGame::set_remaining_time(int time_remaining){
	turn_end_time_ = time(0) + time_remaining;
}

int OmokGame::get_time_remaining(){
	int time_remaining = turn_end_time_ - time(0);
	
	if(time_remaining < 0){
		return 0;
	}
	return time_remaining;
}

int OmokGame::whose_turn(){
	if(is_leaders_turn_){
		return leader_->get_id();
	}
	return guest_->get_id();
}

void OmokGame::switch_turn(){
	is_leaders_turn_ = !is_leaders_turn_;
}

void OmokGame::reset_board(){
	memset(board_, 0, 225);
	is_game_end_ = false;
	who_won_ = 0;
}

bool OmokGame::is_game_end(){
	return is_game_end_;
}

int OmokGame::who_won(){
	return who_won_;
}