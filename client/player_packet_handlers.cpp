#include "interface.hpp"
#include "omok_game.hpp"
#include "omok_game_handler.hpp"
#include "omok_player.hpp"
#include "packet.hpp"
#include "player.hpp"

#include <boost/bind.hpp>

void Player::handle_create_username(){
	//server only
}

void Player::handle_username_result(){
	//client only
	bool success = read<bool>();
	
	//if success, set is_logged_in_ = true;
	if(success){
		is_logged_in_ = true;
		
		id_ = read<int>();
		name_ = ui_info_->username_;
		
		//put the user into the join/create room selection screen
		ui_info_->state_ = UI_JOIN_CREATE_ROOM;
	}
	else{	//else tell the user to use a different name
		name_ = "";
		skip_bytes(4);
	}
	
	//for the interface, the input is no longer cut
	ui_info_->is_input_cut_ = 0;
	
}

//in omok room lobby
void Player::handle_number_of_players_online(){
	//client only
	//show the number of clients connected on the user interface
	int num_online = read<short>();
	ui_info_->num_online_ = num_online;
}

void Player::handle_omok_room_info(){
	//client only
	//add the omok game to the list in the user interface
	int room_id = read<int>();
	std::string room_name = read<std::string>();
	bool is_room_full = read<bool>();
	
	int num_people_in_room = 1;
	if(is_room_full){
		num_people_in_room = 2;
	}
	
	ui_room_list_add_room(ui_info_, room_id, room_name.c_str(), num_people_in_room);
}

void Player::handle_omok_room_update(){
	//client only
	//apply the change to the omok room with the given id to the user interface
	int room_id = read<int>();
	char action = read<char>();
	bool is_room_full = read<bool>();
	
	ui_room_list_update_room(ui_info_, room_id, action, is_room_full);
}

void Player::handle_omok_join_room(){
	//server only
	//check if the omok room requested to be joined is not full
}

void Player::handle_omok_room_unavailable(){
	//client only
	//nothing happens, accept input again
	ui_info_->is_input_cut_ = 0;
}

void Player::handle_omok_create_room(){
	//server only
	//create the omok room
}
	
void Player::handle_omok_room_admittance(){
	//client only
	//update the user interface to show who is the leader,
	//and which room the player is in
	is_in_omok_game_ = true;
	
	ui_info_->state_ = UI_OMOK_GAME;
	ui_info_->omok_messages_size_ = 0;
	ui_omok_game_append_message(ui_info_, "Enter /quit to leave the room, /place <x> <y> to place a piece, and /start to start the game if you are the leader.");
	ui_omok_game_append_message(ui_info_, "You may also use the arrow keys and press End to place a piece during your turn.");
	
	int room_id = read<int>();
	std::string room_name = read<std::string>();
	int leader_id = read<int>();
	std::string leader_name = read<std::string>();
	
	omok_game_ = new OmokGame(room_id, room_name, leader_id, leader_name);
	omok_game_->set_guest(id_, name_);

	//accept input again
	ui_info_->is_input_cut_ = 0;
}

void Player::handle_omok_room_create(){
	//client only
	//update the user interface to show 
	//which room this player is in.
	is_in_omok_game_ = true;
	
	ui_info_->state_ = UI_OMOK_GAME;
	ui_info_->omok_messages_size_ = 0;
	ui_omok_game_append_message(ui_info_, "Enter /quit to leave the room, /place <x> <y> to place a piece, and /start to start the game if you are the leader.");
	ui_omok_game_append_message(ui_info_, "You may also use the arrow keys and press End to place a piece during your turn.");
	
	int room_id = read<int>();
	std::string room_name = read<std::string>();
	
	omok_game_ = new OmokGame(room_id, room_name, id_, name_);
	
	//accept input again
	ui_info_->is_input_cut_ = 0;
}

//in the omok room
void Player::handle_omok_start_game(){
	//server and client receive this
	//server: tell the other player that the game has begun
	//client: set the countdown timer on the omok room interface
	ui_info_->is_omok_game_in_session_ = 1;
	
	omok_game_->reset_board();
	omok_game_->set_remaining_time(30);
}
	
void Player::handle_omok_place_piece(){
	//server and client receive this
	//server: place omok piece and tell the other player about the placement
	//client: the other player placed omok piece.
	//		  and reset remaining time
	
	char x = read<char>();
	char y = read<char>();
	
	omok_game_->set_remaining_time(30);
	
	if(id_ == omok_game_->get_leader()->get_id()){
		//the other player isn't the leader
		omok_game_->place_piece(x, y, false);
	}
	else{
		//the other player is the leader
		omok_game_->place_piece(x, y, true);
	}	
}
	
void Player::handle_omok_win_game(){
	//client only
	//notification that this player has won the omok game
	ui_omok_game_append_message(ui_info_, "You have won the game.");
	
	
	ui_info_->is_omok_game_in_session_ = 0;
}

void Player::handle_omok_lose_game(){
	//client only
	//notification that this player has lost the game
	ui_omok_game_append_message(ui_info_, "You have lost the game.");
	
	ui_info_->is_omok_game_in_session_ = 0;
}

void Player::handle_omok_message(){
	//client and server
	//server: send the other player the message
	//client: show the message in the interface
	std::string sender_name = read<std::string>();
	std::string message = read<std::string>();
	
	std::string full_message = sender_name + ": " + message;
	
	ui_omok_game_append_message(ui_info_, full_message.c_str());
}
	
	
void Player::handle_omok_player_has_joined(){
	//client only
	//show who joined the room
	int guest_id = read<int>();
	std::string guest_name = read<std::string>();
	
	omok_game_->set_guest(guest_id, guest_name);
	
	//show in the interface who joined
	std::string message = guest_name + " has joined the game.";
	ui_omok_game_append_message(ui_info_, message.c_str());
}

void Player::handle_omok_player_has_left(){
	//client only
	//show who left
	int guest_id = read<int>();
	std::string guest_name = read<std::string>();
	
	omok_game_->remove_guest();
	
	//the omok game will have the leader take the first turn
	omok_game_->set_is_leaders_turn(true);

	//reset the board
	omok_game_->reset_board();
	
	//show in the interface who left
	std::string message = guest_name + " has left the game.";
	ui_omok_game_append_message(ui_info_, message.c_str());
	ui_info_->is_omok_game_in_session_ = 0;
}

void Player::handle_omok_switch_turn(){
	//client and server
	//server: send to the other player that the turn is switched
	//client: switch the turns in the omok game. Since switch turns is called by the
	//	  other client when his/her turn is ended, it is certain that it is this user's
	//	  turn now.
	
	//NOTE: switching turns implies that the remaining time should be reset
	
	omok_game_->switch_turn();
	omok_game_->set_remaining_time(30);
	
	//in order to make a callback later if the time runs out
	//make an aynchronous wait operation
	boost::shared_ptr<boost::asio::deadline_timer> t(new boost::asio::deadline_timer(*ui_info_->io_service_, boost::posix_time::milliseconds(30000)));
	t->async_wait(boost::bind(&omok_switch_turn_handler, t, ui_info_));
}

void Player::handle_omok_room_closure(){
	//client only
	//bring the player to the lobby screen
	is_in_omok_game_ = false;
	
	if(omok_game_){
		delete omok_game_;
		omok_game_ = 0;
	}
	
	ui_info_->state_ = UI_JOIN_ROOM;
	ui_info_->is_omok_game_in_session_ = 0;
}

void Player::handle_omok_room_leave(){
	//server only
	//remove the player from the omok room, kicks the other player out
	//if this player is the host
}
	