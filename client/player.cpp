#include "interface.hpp"
#include "omok_game.hpp"
#include "omok_player.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "session.hpp"

#include <iostream>

Player::Player(Session *session, UIInfo *ui_info):
	id_(0),
	read_length_(0),
	name_(""),
	is_logged_in_(false),
	is_in_omok_game_(false),
	omok_game_(0),
	session_(session),
	ui_info_(ui_info)
{
}

template<>
std::string Player::read<std::string>(){
	char *recv = session_->get_recv();
	std::string value = recv + read_length_;
	
	read_length_ += value.length() + 1;
	
	return value;
}

template<>
bool Player::read<bool>(){
	char value = read<char>();
	
	if(value != 0){
		return true;
	}
	return false;
}

void Player::skip_bytes(int n){
	read_length_ += n;
}

Player::~Player(){
	if(omok_game_){
		delete omok_game_;
	}
}

void Player::send(Packet *packet){
	session_->send_packet(packet);
}

void Player::disconnect(){
	session_->disconnect();
}

int Player::get_id(){
	return id_;
}

void Player::set_name(std::string name){
	name_ = name;
}

std::string Player::get_name(){
	return name_;
}

void Player::set_is_logged_in(bool flag){
	is_logged_in_ = flag;
}

bool Player::is_logged_in(){
	return is_logged_in_;
}

void Player::set_is_in_omok_game(bool flag){
	is_in_omok_game_ = flag;
}

bool Player::is_in_omok_game(){
	return is_in_omok_game_;
}

void Player::set_omok_game(OmokGame *omok_game){
	omok_game_ = omok_game;
}

OmokGame *Player::get_omok_game(){
	return omok_game_;
}

void Player::handle_packet(){
	char *recv = session_->get_recv();
	short header = *(short*)(recv);
	short length = *(short*)(recv + 2);
	std::cout << "Player " << id_ << " reading packet with header " << header << " with size " << length << ".\n";
	
	if(is_in_omok_game_){
		handle_packet_in_omok_game();
	}
	else if(is_logged_in_){
		handle_packet_in_lobby();
	}
	else{	//player has not created a name yet
		short packet_header = read<short>();
		skip_bytes(2);
		
		switch(packet_header){
		case 1:	//username creation request from client
			handle_create_username();
			break;
		case 2:	//result of username request
			handle_username_result();
			break;
		default:
			disconnect();
			std::cout << "Disconnect() called in handle_packet()\n";
			break;
		}
	}
	
	read_length_ = 0;
}

void Player::handle_packet_in_lobby(){
	short packet_header = read<short>();
	skip_bytes(2);
	
	switch(packet_header){
	case 3:	//number of players online
		handle_number_of_players_online();
		break;
	case 4: //omok room list
		handle_omok_room_info();
		break;
	case 5: //request to join a room with given name
		handle_omok_join_room();
		break;
	case 6: //request to create a room
		handle_omok_create_room();
		break;
	case 7: //admission into an existing omok room
		handle_omok_room_admittance();
		break;
	case 8: //admission into a new room
		handle_omok_room_create();
		break;
	case 18:
		handle_omok_room_update();
		break;
	case 20:
		handle_omok_room_unavailable();
		break;
	default:
		disconnect();
		std::cout << "Disconnect() called in handle_packet_in_lobby()\n";
		break;
	}
}

void Player::handle_packet_in_omok_game(){
	short packet_header = read<short>();
	skip_bytes(2);
	
	switch(packet_header){
	case 3:	//number of players online
		handle_number_of_players_online();
		break;
	case 4: //omok room list
		handle_omok_room_info();
		break;
	case 9:	//the player chooses to start the omok game
		handle_omok_start_game();
		break;
	case 10:	//the other player places an omok piece
		handle_omok_place_piece();
		break;
	case 11:	//the player wins the game
		handle_omok_win_game();
		break;
	case 12:	//the player loses the game
		handle_omok_lose_game();
		break;
	case 13:	//on client, any player has sent a message
				//on server, the player of this object sent a message
		handle_omok_message();
		break;
	case 14:	//a player has joined the room (client notification)
		handle_omok_player_has_joined();
		break;
	case 15:	//a player has left the room (client notifiction)
		handle_omok_player_has_left();
		break;
	case 16:	//switch the turns
		handle_omok_switch_turn();
		break;
	case 17:	//the omok room is closed (client notification)
		handle_omok_room_closure();
		break;
	case 18:	//one of the rooms in the omok games have changed states/closed
		handle_omok_room_update();
		break;
	case 19:	//request to leave the omok room
		handle_omok_room_leave();
		break;
	default:
		disconnect();
		std::cout << "Disconnect() called in handle_packet_in_omok_game()\n";
		break;
	}
}