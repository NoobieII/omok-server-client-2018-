#include "console_graphics.hpp"
#include "interface.hpp"
#include "key_event.hpp"
#include "omok_game.hpp"
#include "omok_game_handler.hpp"
#include "omok_player.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "session.hpp"

#include <cstring>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void handle_input(UIInfo *ui_info){
	//std::cout << "start ui input\n";
	if(ui_info->is_input_cut_){
		//ignore all incoming input
		while(kbhit()){
			get_char_input();
		}
		return;
	}
	
	while(kbhit()){
		switch(ui_info->state_){
		case UI_ENTER_IP_PORT:
			handle_input_ip_port_screen(ui_info);
			break;
		case UI_ENTER_USERNAME:
			handle_input_username(ui_info);
			break;
		case UI_JOIN_CREATE_ROOM:
			handle_input_join_create_screen(ui_info);
			break;
		case UI_CREATE_ROOM:
			handle_input_create_room_screen(ui_info);
			break;
		case UI_JOIN_ROOM:
			handle_input_join_room_screen(ui_info);
			break;
		case UI_OMOK_GAME:
			handle_input_omok_game_screen(ui_info);
			break;
		}
	}
	//std::cout << "end ui input\n";
}

//first screen, user is on the screen where the ip and port
//in the text box before making the connection
//will throw an error if the ip can't be resolved
void handle_input_ip_port_screen(UIInfo *ui_info){
	char input = get_char_input();
	
	if(input == KeyEvents::ENT){
		if(ui_info->ip_address_port_cursor_position_ == 0){
			return;
		}
		char ip[16], port[16];
		
		//expect at least 2 strings
		if(sscanf(ui_info->ip_address_port_, "%s%s", ip, port) != 2){
			return;
		}
		
		//attempt to make a connection
		try{
			boost::asio::io_service &io = *ui_info->io_service_;
			std::string ip_str = ip;
			std::string port_str = port;
			
			boost::asio::ip::tcp::resolver resolver(io);
			boost::asio::ip::tcp::resolver::query query(ip_str, port_str);
			boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
			boost::asio::ip::tcp::endpoint endpoint = *iterator;
			
			
			ui_info->session_->get_socket().connect(endpoint);
			ui_info->session_->initialize();
			
			//put the user into the join/create screen
			ui_info->state_ = UI_ENTER_USERNAME;
		}
		catch(std::exception &e){
			ui_info->error_ = CONNECTION_FAIL;
		}
		
		return;
	}
	else if(input == KeyEvents::BKS){
		if(ui_info->ip_address_port_cursor_position_ == 0){
			return;
		}
		
		ui_info->ip_address_port_cursor_position_--;
		ui_info->ip_address_port_[ui_info->ip_address_port_cursor_position_] = '\0';
	}
	//input a regular ascii character
	else if(input > 31 && ui_info->ip_address_port_cursor_position_ < 31){
		ui_info->ip_address_port_[ui_info->ip_address_port_cursor_position_] = input;
		ui_info->ip_address_port_cursor_position_++;
	}
}

//after making a connection to the server, the user
//will enter the username
void handle_input_username(UIInfo *ui_info){
	char input = get_char_input();
	
	if(input == KeyEvents::BKS){
		if(ui_info->username_cursor_position_ > 0){
			ui_info->username_cursor_position_--;
			ui_info->username_[ui_info->username_cursor_position_] = '\0';
		}
		else{
			return;
		}
	}
	if(input == KeyEvents::ENT){
		//check if the string is at least 2 characters long
		if(ui_info->username_cursor_position_ >= 2){
			//send a packet to the server to check for the username availability
			Packet packet1;
			packet1.create_username(std::string(ui_info->username_));
			ui_info->session_->send_packet(&packet1);
			
			//block all incoming input until the server sent back a response
			ui_info->is_input_cut_ = 1;
			
			//the packet handler will revert is_input_cut_ to 0 when receiving
			//a username_result packet
		}
		else{
			return;
		}
	}
	//enter a normal ascii character
	if(input > 31){
		if(ui_info->username_cursor_position_ < 15){
			ui_info->username_[ui_info->username_cursor_position_] = input;
			ui_info->username_cursor_position_++;
			return;
		}
	}
}

//after getting an approval for the username entered,
//select between joining a room or creating a room
void handle_input_join_create_screen(UIInfo *ui_info){
	char input = get_char_input();
	
	if(input == KeyEvents::KU || input == KeyEvents::KD){
		ui_info->join_create_cursor_position_++;
		ui_info->join_create_cursor_position_ %= 2;
		return;
	}
	if(input == KeyEvents::ENT){
		//create room
		if(ui_info->join_create_cursor_position_ == 0){
			ui_info->state_ = UI_JOIN_ROOM;
		}
		if(ui_info->join_create_cursor_position_ == 1){
			ui_info->state_ = UI_CREATE_ROOM;
		}
		return;
	}		
}

//in the screen that shows the list of Omok games open
void handle_input_join_room_screen(UIInfo *ui_info){
	char input = get_char_input();
	
	if(input == KeyEvents::ESC){
		//go back to the join/create screen
		ui_info->state_ = UI_JOIN_CREATE_ROOM;
		return;
	}
	if(input == KeyEvents::KU){
		if(ui_info->room_list_cursor_position_ > 0){
			ui_info->room_list_cursor_position_--;
		}
		return;
	}
	if(input == KeyEvents::KD){
		if(ui_info->room_list_cursor_position_ < ui_info->room_list_size_ - 1){
			ui_info->room_list_cursor_position_++;
		}
		return;
	}
	if(input == KeyEvents::ENT){
		//player sends a packet to the server requesting to
		//join the room
		int room_id = ui_info->room_list_ids_[ui_info->room_list_cursor_position_];
		
		Packet packet5;
		packet5.omok_join_room(room_id);
		ui_info->session_->send_packet(&packet5);
		
		//wait for a response
		ui_info->is_input_cut_ = 1;
	}
}

//creating a room
void handle_input_create_room_screen(UIInfo *ui_info){
	char input = get_char_input();
	
	if(input == KeyEvents::ESC){
		ui_info->state_ = UI_JOIN_CREATE_ROOM;
		return;
	}
	if(input == KeyEvents::BKS){
		if(ui_info->create_room_name_cursor_position_ > 0){
			ui_info->create_room_name_cursor_position_--;
			ui_info->create_room_name_[ui_info->create_room_name_cursor_position_] = '\0';
		}
		return;
	}
	if(input == KeyEvents::ENT){
		if(ui_info->create_room_name_cursor_position_ > 0){
			//send a packet to the server requesting the creation of a room
			Packet packet6;
			packet6.omok_create_room(std::string(ui_info->create_room_name_));
			ui_info->session_->send_packet(&packet6);
			
			ui_info->is_input_cut_ = 1;
		}
		return;
	}
	if(input > 31){
		//append a character to the 
		if(ui_info->create_room_name_cursor_position_ < 15){
			ui_info->create_room_name_[ui_info->create_room_name_cursor_position_] = input;
			ui_info->create_room_name_cursor_position_++;
		}
		return;
	}
}
	
void handle_input_omok_game_screen(UIInfo *ui_info){
	char input = get_char_input();
	
	//it will be a texting type interface
	
	if(input == KeyEvents::BKS){
		//delete a character in the text box
		if(ui_info->incomplete_omok_message_cursor_position_ > 0){
			ui_info->incomplete_omok_message_cursor_position_--;
			ui_info->incomplete_omok_message_[ui_info->incomplete_omok_message_cursor_position_] = '\0';
		}
		return;
	}
	if(input == KeyEvents::ENT){
		//parse the input, whether it is to send a message
		//or to do an action such as place a piece
		
		//current commands: /quit, /place <x> <y>, /start
		
		//check the length of the input first
		if(strlen(ui_info->incomplete_omok_message_) < 1){
			return;
		}
		
		
		Player *player = ui_info->session_->get_player();
		OmokGame *omok_game = player->get_omok_game();
		
		//make a buffer for checking the first word in the message to
		//see if it is a command
		char first_word[128];
		
		sscanf(ui_info->incomplete_omok_message_, "%s", first_word);
		
		if(strcmp(first_word, "/start") == 0){
			//start the omok game
			
			//check if the player is the leader and that there is a guest
			if(omok_game->get_leader()->get_id() != player->get_id() || !omok_game->get_guest()){
				return;
			}
			
			ui_info->is_omok_game_in_session_ = 1;
			omok_game->reset_board();
			omok_game->set_remaining_time(30);
			
			//in order to make a callback later if the time runs out
			//make an aynchronous wait operation
			boost::shared_ptr<boost::asio::deadline_timer> t(new boost::asio::deadline_timer(*ui_info->io_service_, boost::posix_time::milliseconds(30000)));
			t->async_wait(boost::bind(&omok_switch_turn_handler, t, ui_info));
			
			//tell the other player that the omok game is starting
			Packet packet9;
			packet9.omok_start_game();
			player->send(&packet9);
		}
		else if(strcmp(first_word, "/place") == 0){
			//attempt to place a piece
			
			//check if it is the player's turn, if there are two players inside and that the game is started
			if(player->get_id() != omok_game->whose_turn() || !omok_game->get_guest() || !ui_info->is_omok_game_in_session_){
				return;
			}
			
			//check if the syntax of the placement command is correct
			int x, y;
			if(sscanf(ui_info->incomplete_omok_message_, "%*s%x%x", &x, &y) != 2){
				return;
			}
			
			//do bounds checking for the coordinate pair
			if(x > 14 || y > 14){
				return;
			}
			
			//attempt to place the omok piece
			bool is_leader = omok_game->get_leader()->get_id() == player->get_id();
			
			if(omok_game->place_piece(x, y, is_leader) == 0){
				ui_omok_game_append_message(ui_info, "You cannot place it there.");
				return;
			}
			
			//legal move, so switch turns, reset the timer to 30
			omok_game->switch_turn();
			omok_game->set_remaining_time(30);
			
			Packet packet10;
			packet10.omok_place_piece(x, y);
			player->send(&packet10);
			
			Packet packet16;
			packet16.omok_switch_turn();
			player->send(&packet16);
		}
		else if(strcmp(first_word, "/quit") == 0){
			//leave the room
			player->set_is_in_omok_game(false);
			ui_info->state_ = UI_JOIN_ROOM;
			ui_info->is_omok_game_in_session_ = 0;
			
			delete omok_game;
			player->set_omok_game(0);
			
			//send to the server that the player left
			Packet packet19;
			packet19.omok_room_leave();
			ui_info->session_->send_packet(&packet19);
		}
		else{
			//the text typed in is a message
			//send the message to the other player
			std::string self_message = player->get_name() + ": " + ui_info->incomplete_omok_message_;
			
			ui_omok_game_append_message(ui_info, self_message.c_str());
			
			std::string message = ui_info->incomplete_omok_message_;
			
			Packet packet13;
			packet13.omok_message(player->get_name(), message);
			ui_info->session_->send_packet(&packet13);
		}
		
		//remove what is typed in the text box
		memset(ui_info->incomplete_omok_message_, 0, 128);
		ui_info->incomplete_omok_message_cursor_position_ = 0;
		
		return;
	}
	if(input > 31){
		//append character to text box
		if(ui_info->incomplete_omok_message_cursor_position_ < 127){
			ui_info->incomplete_omok_message_[ui_info->incomplete_omok_message_cursor_position_] = input;
			ui_info->incomplete_omok_message_cursor_position_++;
		}
		return;
	}
	if(input == KeyEvents::KU){
		//move the cursor up
		ui_info->is_omok_piece_cursor_activated_ = 1;
		
		if(ui_info->omok_y_cursor_position_ > 0){
			ui_info->omok_y_cursor_position_--;
		}
		return;
	}
	if(input == KeyEvents::KD){
		//move the cursor down
		ui_info->is_omok_piece_cursor_activated_ = 1;
		
		if(ui_info->omok_y_cursor_position_ < 14){
			ui_info->omok_y_cursor_position_++;
		}
		return;
	}
	if(input == KeyEvents::KL){
		//move the cursor left
		ui_info->is_omok_piece_cursor_activated_ = 1;
		
		if(ui_info->omok_x_cursor_position_ > 0){
			ui_info->omok_x_cursor_position_--;
		}
		return;
	}
	if(input == KeyEvents::KR){
		//move the cursor right
		ui_info->is_omok_piece_cursor_activated_ = 1;
		
		if(ui_info->omok_x_cursor_position_ < 14){
			ui_info->omok_x_cursor_position_++;
		}
		return;
	}
	if(input == KeyEvents::END){
		Player *player = ui_info->session_->get_player();
		OmokGame *omok_game = player->get_omok_game();
		
		//check if it is the player's turn, if there are two players inside and that the game is started
		if(player->get_id() != omok_game->whose_turn() || !omok_game->get_guest() || !ui_info->is_omok_game_in_session_){
			return;
		}
		
		//try to place a piece
		
		int x = ui_info->omok_x_cursor_position_;
		int y = ui_info->omok_y_cursor_position_;
		
		bool is_leader = omok_game->get_leader()->get_id() == player->get_id();
		
		if(omok_game->place_piece(x, y, is_leader) == 0){
			ui_omok_game_append_message(ui_info, "You cannot place it there.");
			return;
		}
		
		ui_info->is_omok_piece_cursor_activated_ = 0;
		
		//legal move, so switch turns, reset the timer to 30
		omok_game->switch_turn();
		omok_game->set_remaining_time(30);
		
		Packet packet10;
		packet10.omok_place_piece(x, y);
		player->send(&packet10);
		
		Packet packet16;
		packet16.omok_switch_turn();
		player->send(&packet16);
		
		return;
	}
}


void draw_ui(UIInfo *ui_info){
	//std::cout << "start draw ui\n";
	switch(ui_info->state_){
	case UI_ENTER_IP_PORT:
		draw_ui_ip_port_screen(ui_info);
		break;
	case UI_ENTER_USERNAME:
		draw_ui_username(ui_info);
		break;
	case UI_JOIN_CREATE_ROOM:
		draw_ui_join_create_screen(ui_info);
		break;
	case UI_CREATE_ROOM:
		draw_ui_create_room_screen(ui_info);
		break;
	case UI_JOIN_ROOM:
		draw_ui_join_room_screen(ui_info);
		break;
	case UI_OMOK_GAME:
		draw_ui_omok_game_screen(ui_info);
		break;
	}
	
	render();
	//std::cout << "end draw ui\n";
}

void draw_ui_ip_port_screen(UIInfo *ui_info){
	draw_str("Welcome to Ryan's Omok game server.\n\rType in the Ip address and port.", 20, 5, 100, 2);
	draw_area('_', 20, 10, 32, 1);
	
	draw_str(ui_info->ip_address_port_, 20, 10);
}

void draw_ui_username(UIInfo *ui_info){
	draw_str("Enter a username.", 20, 5);
	draw_area('_', 20, 10, 16, 1);
	
	draw_str(ui_info->username_, 20, 10);
}

void draw_ui_join_create_screen(UIInfo *ui_info){
	draw_str(ui_info->username_, 0, 0);
	draw_str("Players Online:", 50, 0);
	draw_int_left_end(ui_info->num_online_, 66, 0);
	
	draw_str("Join Omok room", 20, 10);
	draw_str("Create room", 20, 11);
	
	//draw the cursor over the join room option
	if(ui_info->join_create_cursor_position_ == 0){
		draw_str(">", 19, 10);
	}
	else{
		draw_str(">", 19, 11);
	}
}

void draw_ui_join_room_screen(UIInfo *ui_info){
	draw_str(ui_info->username_, 0, 0);
	draw_str("Players Online:", 50, 0);
	draw_int_left_end(ui_info->num_online_, 66, 0);
	
	draw_str("Room name", 5, 5);
	draw_str("Vacancy", 30, 5);
	
	//draw the available rooms
	for(int i = 0; i < ui_info->room_list_size_; ++i){
		draw_str(ui_info->room_list_names_[i], 5, 7 + i);
		draw_int_left_end(ui_info->room_list_num_players_[i], 30, 7 + i);
		
		draw_str("/2", 31, 7 + i);
	}
	
	//draw the cursor
	draw_str(">", 4, 7 + ui_info->room_list_cursor_position_);
}
	
void draw_ui_create_room_screen(UIInfo *ui_info){
	draw_str(ui_info->username_, 0, 0);
	draw_str("Players Online:", 50, 0);
	draw_int_left_end(ui_info->num_online_, 66, 0);
	
	draw_str("Create an omok game with a room name.", 20, 5);
	
	draw_area('_', 20, 10, 16, 1);
	draw_str(ui_info->create_room_name_, 20, 10);
}

void draw_ui_omok_game_screen(UIInfo *ui_info){
	draw_str(ui_info->username_, 0, 0);
	draw_str("Players Online:", 50, 0);
	draw_int_left_end(ui_info->num_online_, 66, 0);
	
	OmokGame *omok_game = ui_info->session_->get_player()->get_omok_game();
	
	draw_str(omok_game->get_name().c_str(), 0, 3);
	
	//draw the board
	draw_text("0123456789ABCDE->x", 2, 5, 18, 1);
	draw_text("0123456789ABCDE|y", 0, 7, 1, 17);
	
	draw_area('+', 1, 6, 1, 1);
	draw_area('+', 1, 22, 1, 1);
	draw_area('+', 17, 6, 1, 1);
	draw_area('+', 17, 22, 1, 1);
	draw_area('-', 2, 6, 15, 1);
	draw_area('-', 2, 22, 15, 1);
	draw_area('|', 1, 7, 1, 15);
	draw_area('|', 17, 7, 1, 15);
	
	enum PlayerType{LEADER = 1, GUEST};
	
	const char *board = omok_game->get_board();
	
	//X represents the leader's pieces and O represents the guest's pieces
	for(int j = 0; j < 15; j++){
		for(int i = 0; i < 15; i++){
			if(board[j * 15 + i] == LEADER){
				//draw_area(249, 2 + i, 7 + j, 1, 1);
				draw_str("+", 2 + i, 7 + j);
			}
			if(board[j * 15 + i] == GUEST){
				draw_str("O", 2 + i, 7 + j);
			}
		}
	}
	
	//draw the host and guest info
	draw_str("Host", 29, 3);
	draw_str(omok_game->get_leader()->get_name().c_str(), 29, 5);
	
	draw_str("Guest", 50, 3);
	if(omok_game->get_guest()){
		draw_str(omok_game->get_guest()->get_name().c_str(), 50, 5);
	}
	
	//draw the messages, within a 60x20 box
	//Bambo (aka FPBambo) recommended that I make the messages appear from bottom up
	int row_offset = 0;
	for(int i = 0; i < ui_info->omok_messages_size_ && i + row_offset < 20; ++i){
		int message_length = strlen(ui_info->omok_messages_[i]);
		row_offset += message_length / 60;
		
		draw_str(ui_info->omok_messages_[i], 19, 27 - (row_offset + i), 60, 20);
	}
	
	//draw the incomplete message of the user
	int incomplete_message_length = strlen(ui_info->incomplete_omok_message_);
	
	//scroll the message to the left if it goes off screen
	if(incomplete_message_length > 60){
		draw_str(ui_info->incomplete_omok_message_ + incomplete_message_length - 60, 19, 29, 60, 1);
	}
	else{
		draw_str(ui_info->incomplete_omok_message_, 19, 29, 60, 1);
	}
	
	//if the omok game is in session, there are some additional things to render
	if(!ui_info->is_omok_game_in_session_){
		return;
	}
	
	//tell whose turn it is
	std::string turn_str;
	if(omok_game->whose_turn() == omok_game->get_leader()->get_id()){
		turn_str = "It is " + omok_game->get_leader()->get_name() + "\'s turn.";
	}
	else{
		turn_str = "It is " + omok_game->get_guest()->get_name() + "\'s turn.";
	}
	draw_str_wrapped(turn_str.c_str(), 0, 24, 19, 5);
	
	//tell the remaining time
	draw_str("Remaining time: ", 0, 26);
	draw_int_left_end(omok_game->get_time_remaining(), 16, 26);
	
	//draw the cursor on the board
	if(ui_info->is_omok_piece_cursor_activated_){
		Sprite cursor(0, 1, 1, "*\0", true, true, 1, 2);
		
		draw_image(&cursor, 2 + ui_info->omok_x_cursor_position_, 7 + ui_info->omok_y_cursor_position_);
	}
}

//add a room to the list in the interface
void ui_room_list_add_room(UIInfo *ui_info, int id, const char *name, int num_players){
	//there is no more space for the room list
	if(ui_info->room_list_size_ >= 16){
		return;
	}
	
	int i = ui_info->room_list_size_;
	
	ui_info->room_list_ids_[i] = id;
	strcpy(ui_info->room_list_names_[i], name);
	ui_info->room_list_num_players_[i] = num_players;
	
	ui_info->room_list_size_++;
}

void ui_room_list_update_room(UIInfo *ui_info, int id, int action, bool is_full){
	if(action == 1){	//change the room's status
		for(int i = 0; i < ui_info->room_list_size_; ++i){
			if(ui_info->room_list_ids_[i] == id){
				ui_info->room_list_num_players_[i] = 1 + static_cast<char>(is_full);
				break;
			}
		}
	}
	if(action == 2){	//remove the room from the list
		if(ui_info->room_list_size_ == 0){
			return;
		}
		
		int i = 0;
		//find the index of the room to remove from the list
		while(i < ui_info->room_list_size_){
			if(ui_info->room_list_ids_[i] == id){
				break;
			}
			++i;
		}
		
		//copy the list contents one space back
		while(i < ui_info->room_list_size_ - 1){
			strcpy(ui_info->room_list_names_[i], ui_info->room_list_names_[i + 1]);
			ui_info->room_list_ids_[i] = ui_info->room_list_ids_[i + 1];
			ui_info->room_list_num_players_[i] = ui_info->room_list_num_players_[i + 1];
			
			++i;
		}
		
		ui_info->room_list_size_--;
		
	}
}

void ui_omok_game_append_message(UIInfo *ui_info, const char *message){
	//move the messages up one space
	for(int i = ui_info->omok_messages_size_; i > 0; --i){
		if(i < 20){
			memcpy(ui_info->omok_messages_[i], ui_info->omok_messages_[i - 1], 128);
		}
	}
	strcpy(ui_info->omok_messages_[0], message);
	
	if(ui_info->omok_messages_size_ < 20){
		ui_info->omok_messages_size_++;
	}
}