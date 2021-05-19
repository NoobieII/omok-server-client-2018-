#include "omok_game.hpp"
#include "omok_player.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "server.hpp"

void Player::handle_create_username(){
	//server only
	std::string name = read<std::string>();
	
	//check if the username is not taken
	int id = Server::get_instance()->get_player_id_by_name(name);
	
	if(id == 0){	//the name isn't taken
		name_ = name;
		is_logged_in_ = true;
		id_ = Server::get_instance()->get_player_id();
		
		Server::get_instance()->add_player(id_, this);
		Server::get_instance()->add_player_name(id_, name_);
		
		
		//send the client that the name is given successfully
		Packet packet2;
		packet2.username_result(true, id_);
		
		send(&packet2);
		
		//send the client the list of current rooms
		std::map<int, OmokGame*> *omok_games = Server::get_instance()->get_omok_games();
		std::map<int, OmokGame*>::iterator it = omok_games->begin();
		
		while(it != omok_games->end()){
			OmokGame *game = it->second;
			//send a packet for each of the current omok games
			if(game){
				Packet packet4;
				packet4.omok_room_info(game->get_id(), game->get_name(), game->get_guest());
				send(&packet4);
			}
			it++;
		}
		
		//send the players online an updated online population
		Packet packet3;
		packet3.number_of_players_online(Server::get_instance()->get_num_players_online());
		Server::get_instance()->send(&packet3);
	}
	else{
		//send the client that the name can't be taken
		Packet packet2;
		packet2.username_result(false, 0);
		send(&packet2);
	}
}

void Player::handle_username_result(){
	//client only
	//if success, set in_lobby_ = true;
	//else tell the user to use a different name
}

//in omok room lobby
void Player::handle_number_of_players_online(){
	//client only
	//show the number of clients connected on the user interface
}

void Player::handle_omok_room_info(){
	//client only
	//add the omok game to the list in the user interface
}

void Player::handle_omok_room_update(){
	//client only
	//apply the change to the omok room with the given id to the user interface
}

void Player::handle_omok_join_room(){
	//server only
	//check if the omok room requested to be joined is not full
	int room_id = read<int>();
	
	OmokGame *game = Server::get_instance()->get_omok_game(room_id);
	
	if(!game){
		Packet packet20;
		packet20.omok_room_unavailable();
		send(&packet20);
		
		return;
	}
	//if the game already has a guest inside
	if(game->get_guest()){
		Packet packet20;
		packet20.omok_room_unavailable();
		send(&packet20);
		
		return;
	}
	
	omok_game_ = game;
	omok_game_->set_guest(id_, name_);
	
	//give this player the info needed to be in the room
	Packet packet7;
	packet7.omok_room_admittance(omok_game_);
	send(&packet7);
	
	//tell the other player that this player has joined
	Player *host = Server::get_instance()->get_player(omok_game_->get_leader()->get_id());
	
	Packet packet14;
	packet14.omok_player_has_joined(this);
	host->send(&packet14);
	
	//send to the rest of the players online the change of the room's status
	Packet packet18;
	packet18.omok_room_update(omok_game_->get_id(), 1, true);
	Server::get_instance()->send(&packet18);
	
	is_in_omok_game_ = true;
}

void Player::handle_omok_create_room(){
	//server only
	//create the omok room
	Server *server = Server::get_instance();
	int omok_room_id = server->get_omok_room_id();
	std::string room_name = read<std::string>();
	
	server->add_omok_game(omok_room_id, room_name, this);
	
	//put the player in the newly created room
	omok_game_ = server->get_omok_game(omok_room_id);
	
	//give the player the info needed to be in the room
	Packet packet8;
	packet8.omok_room_create(omok_game_);
	send(&packet8);
	
	//tell all others online that there is a new room
	Packet packet4;
	packet4.omok_room_info(omok_room_id, room_name, false);
	server->send(&packet4);
	
	is_in_omok_game_ = true;
}
	
void Player::handle_omok_room_admittance(){
	//client only
	//update the user interface to show who is the leader,
	//and which room the player is in
}

void Player::handle_omok_room_create(){
	//client only
	//update the user interface to show 
	//which room this player is in.
}

//in the omok room
void Player::handle_omok_start_game(){
	//server and client receive this
	//server: tell the other player that the game has begun
	//client: set the countdown timer on the omok room interface
	
	omok_game_->reset_board();
	omok_game_->set_remaining_time(30);
	omok_game_->switch_turn();
	
	Player *guest = Server::get_instance()->get_player(omok_game_->get_guest()->get_id());
	
	Packet packet9;
	packet9.omok_start_game();
	guest->send(&packet9);
	
	Packet packet16;
	packet16.omok_switch_turn();
	send(&packet16);
	guest->send(&packet16);
}
	
void Player::handle_omok_place_piece(){
	//server and client receive this
	//server: place omok piece and tell the other player about the placement
	//		and switch turns
	//client: place omok piece to the board and reset remaining time
	
	char x = read<char>();
	char y = read<char>();
	
	Player *other_player;
	
	//place piece on the omok board
	if(id_ == omok_game_->get_leader()->get_id()){
		//the player is leader, place piece
		omok_game_->place_piece(x, y, true);
		
		other_player = Server::get_instance()->get_player(omok_game_->get_guest()->get_id());
	}
	else{
		omok_game_->place_piece(x, y, false);
		
		other_player = Server::get_instance()->get_player(omok_game_->get_leader()->get_id());
	}
	
	//send packet about the placement
	Packet packet10;
	packet10.omok_place_piece(x, y);
	other_player->send(&packet10);
	
	//if the game has ended, send each player
	//whether they won or lost
	if(omok_game_->is_game_end()){
		Packet packet11;
		packet11.omok_win_game();
		
		Packet packet12;
		packet12.omok_lose_game();
		
		//if this player won
		if(id_ == omok_game_->who_won()){
			send(&packet11);
			other_player->send(&packet12);
		}
		//if the other player won
		else{
			send(&packet12);
			other_player->send(&packet11);
		}
	}
}
	
void Player::handle_omok_win_game(){
	//client only
	//notification that this player has won the omok game
}

void Player::handle_omok_lose_game(){
	//client only
	//notification that this player has lost the game
}

void Player::handle_omok_message(){
	//client and server
	//server: send the other player the message
	//client: show the message in the interface
	std::string sender_name = read<std::string>();
	std::string message = read<std::string>();
	
	//if there is no one else in the room
	if(!omok_game_->get_guest()){
		return;
	}
	
	Player *other_player;
	
	if(id_ == omok_game_->get_leader()->get_id()){
		other_player = Server::get_instance()->get_player(omok_game_->get_guest()->get_id());
	}
	else{
		other_player = Server::get_instance()->get_player(omok_game_->get_leader()->get_id());
	}
	
	Packet packet13;
	packet13.omok_message(sender_name, message);
	other_player->send(&packet13);
}
	
	
void Player::handle_omok_player_has_joined(){
	//client only
	//show who joined the room
}

void Player::handle_omok_player_has_left(){
	//client only
	//show who left
}

void Player::handle_omok_switch_turn(){
	//client and server
	//server: send to the other player that the turn is switched
	//client: switch the turns in the omok game
	
	//NOTE: switching turns implies that the remaining time should be reset
	
	omok_game_->switch_turn();
	omok_game_->set_remaining_time(30);
	
	Player *other_player;
	
	if(id_ == omok_game_->get_leader()->get_id()){
		other_player = Server::get_instance()->get_player(omok_game_->get_guest()->get_id());
	}
	else{
		other_player = Server::get_instance()->get_player(omok_game_->get_leader()->get_id());
	}
	
	Packet packet16;
	packet16.omok_switch_turn();
	other_player->send(&packet16);
}

void Player::handle_omok_room_closure(){
	//client only
	//bring the player to the lobby screen
}

void Player::handle_omok_room_leave(){
	//server only
	//remove the player from the omok room, kicks the other player out
	//if this player is the host
	is_in_omok_game_ = false;
	
	if(id_ == omok_game_->get_leader()->get_id()){
		//the player is the leader
		
		//if there is a guest, tell him/her that the room is closed
		if(omok_game_->get_guest()){
			Player *other_player = Server::get_instance()->get_player(omok_game_->get_guest()->get_id());
			
			Packet packet17;
			packet17.omok_room_closure();
			other_player->send(&packet17);
			
			//set the other player's variables
			omok_game_->remove_guest();
			other_player->set_is_in_omok_game(false);
			other_player->set_omok_game(0);
		}
		
		//send an updated status of the omok game
		Packet packet18;
		packet18.omok_room_update(omok_game_->get_id(), 2, false);
		Server::get_instance()->send(&packet18);
		
		//remove the omok game
		Server::get_instance()->remove_omok_game(omok_game_->get_id());
		omok_game_ = 0;
		
		return;
	}
	else{
		//the player is the guest, or at least should be the guest
		if(omok_game_->get_guest() && id_ == omok_game_->get_guest()->get_id()){
			//tell the leader that the player is leaving
			Player *other_player = Server::get_instance()->get_player(omok_game_->get_leader()->get_id());
			
			Packet packet15;
			packet15.omok_player_has_left(this);
			other_player->send(&packet15);
			
			//send an updated status of the omok game
			Packet packet18;
			packet18.omok_room_update(omok_game_->get_id(), 1, false);
			Server::get_instance()->send(&packet18);
			
			//set the variables
			omok_game_->remove_guest();
			omok_game_ = 0;
		}
	}
}	
	/*
	if(id_ == omok_game_->get_leader()->get_id()){
		is_leader = true;
		other_player = Server::get_instance()->get_player(omok_game_->get_guest()->get_id());
	}
	else{
		is_leader = false;
		other_player = Server::get_instance()->get_player(omok_game_->get_leader()->get_id());
	}
	
	if(is_leader){
		//kick the other player out
		Packet packet17;
		packet17.omok_room_closure();
		other_player->send(&packet17);
		
		other_player->set_omok_game(0);
		other_player->set_is_in_omok_game(false);
		
		//update the omok game info for all players
		Packet packet18;
		packet18.omok_room_update(omok_game_->get_id(), 2, false);
		Server::get_instance()->send(&packet18);
		
		//remove the instance of the omok game
		Server::get_instance()->remove_omok_game(omok_game_->get_id());
		
		omok_game_ = 0;
	}
	else{
		//it is the guest leaving, tell the other player that the guest left
		
		Packet packet15;
		packet15.omok_player_has_left(this);
		other_player->send(&packet15);
		
		//give updated room status to all players
		Packet packet18;
		packet18.omok_room_update(omok_game_->get_id(), 1, false);
		Server::get_instance()->send(&packet18);
		
		omok_game_->remove_guest();
		omok_game_ = 0;
	}
	*/

	