#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <boost/asio.hpp>

class OmokGame;
class Session;

enum UIState{
	UI_ENTER_IP_PORT, UI_ENTER_USERNAME, UI_JOIN_CREATE_ROOM, UI_CREATE_ROOM,
	UI_JOIN_ROOM, UI_OMOK_GAME
};

enum UIError{
	CONNECTION_FAIL, USERNAME_TAKEN, ROOM_CLOSED
};

struct UIInfo{
	int state_;
	int error_;
	char is_input_cut_;	//set to non zero value when the input has to be ignored
	Session *session_;
	boost::asio::io_service *io_service_;
	
	char ip_address_port_cursor_position_;
	char ip_address_port_[32];
	
	char username_cursor_position_;
	char username_[16];
	
	char num_online_;
	
	char join_create_cursor_position_;
	
	char create_room_name_cursor_position_;
	char create_room_name_[16];
	
	char room_list_size_;
	char room_list_cursor_position_;
	int room_list_ids_[16];
	char room_list_names_[16][16];
	char room_list_num_players_[16];
	
	char incomplete_omok_message_cursor_position_;
	char incomplete_omok_message_[128];
	char omok_messages_size_;
	char omok_messages_[20][128];
	char is_omok_game_in_session_;
	
	char is_omok_piece_cursor_activated_;
	char omok_x_cursor_position_;
	char omok_y_cursor_position_;
};

void handle_input(UIInfo *ui_info);
void handle_input_ip_port_screen(UIInfo *ui_info);
void handle_input_username(UIInfo *ui_info);
void handle_input_join_create_screen(UIInfo *ui_info);
void handle_input_join_room_screen(UIInfo *ui_info);
void handle_input_create_room_screen(UIInfo *ui_info);
void handle_input_omok_game_screen(UIInfo *ui_info);

//interface functions
void ui_room_list_add_room(UIInfo *ui_info, int id, const char *name, int num_players);
void ui_room_list_update_room(UIInfo *ui_info, int id, int action, bool is_full);
void ui_omok_game_append_message(UIInfo *ui_info, const char *message);

void draw_ui(UIInfo *ui_info);
void draw_ui_ip_port_screen(UIInfo *ui_info);
void draw_ui_username(UIInfo *ui_info);
void draw_ui_join_create_screen(UIInfo *ui_info);
void draw_ui_join_room_screen(UIInfo *ui_info);
void draw_ui_create_room_screen(UIInfo *ui_info);
void draw_ui_omok_game_screen(UIInfo *ui_info);

#endif