#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>
#include <cstring>

const size_t PACKET_SIZE = 16;
const size_t PACKET_INCREASE_SIZE = 256;

class OmokGame;
class Player;

class Packet{
public:
	Packet();
	~Packet();
	
	//first menu screen
	void create_username(std::string username);
	void username_result(bool success, int id);
	
	void number_of_players_online(int num_online);
	
	//in the omok join lobby
	void omok_room_info(int id, std::string name, bool is_full);
	void omok_room_update(int id, char action, bool is_full);
	void omok_join_room(int room_id);
	void omok_room_unavailable();					//tells a client that the attempted room to join
													//is unavailable
	void omok_create_room(std::string room_name);	//tells the server what the name of the room is to be created
	
	//puts a player into omok room
	void omok_room_admittance(OmokGame *game);	//another player is the leader
	void omok_room_create(OmokGame *game);		//the player receiving this packet is the leader
	
	//within the omok game
	void omok_start_game();							//sent by leader, server signals both clients that the game is started
	void omok_place_piece(int x, int y);			//request/notification that a piece is placed
	void omok_win_game();							//notify the player won the game
	void omok_lose_game();							//notify the player lost
	void omok_message(std::string player_name, std::string message);			//message sent and received in omok room
	void omok_player_has_joined(Player *player);	//notification that a player joined
	void omok_player_has_left(Player *player);		//notification that a player left
	void omok_switch_turn();						//tells client whose turn it is now
	void omok_room_closure();						//notification that the room is closed
	void omok_room_leave();							//tells the server that the player left the room

	char *get();
	size_t size();
private:
	//template for writing integer types
	template<typename TYPE>
	void write(TYPE data){
		//check if the buffer has to be resized
		if(sizeof(TYPE) + length_ >= size_){
			size_ += PACKET_INCREASE_SIZE;
			char *new_buffer = new char[size_];
			
			memcpy(new_buffer, buffer_, length_);
			
			delete[] buffer_;
			buffer_ = new_buffer;
		}
		//write the contents
		*(TYPE*)(buffer_ + length_) = data;
		length_ += sizeof(TYPE);
	}
	
	size_t size_;
	size_t length_;
	char *buffer_;
};

//template specialization for string and bool types
template<>
void Packet::write<std::string>(std::string data);
template<>
void Packet::write<bool>(bool data);

#endif