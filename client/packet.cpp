#include "omok_game.hpp"
#include "omok_player.hpp"
#include "packet.hpp"
#include "player.hpp"

#include <map>

template<>
void Packet::write<std::string>(std::string data){
	//check if the buffer has to be resized
	//size determined by the length of the string itself, including null character
	if(strlen(data.c_str()) + 1 + length_ >= size_){
		size_ += PACKET_INCREASE_SIZE;
		char *new_buffer = new char[size_];
		
		memcpy(new_buffer, buffer_, length_);
		
		delete[] buffer_;
		buffer_ = new_buffer;
	}
	//write the string
	memcpy(buffer_ + length_, data.c_str(), strlen(data.c_str()) + 1);
	length_ += strlen(data.c_str()) + 1;
}

template<>
void Packet::write<bool>(bool data){
	if(data){
		write<char>(1);
	}
	else{
		write<char>(0);
	}
}

Packet::Packet():
	size_(PACKET_SIZE),
	length_(0),
	buffer_(new char[PACKET_SIZE]())
{
}

Packet::~Packet(){
	delete[] buffer_;
}

void Packet::create_username(std::string username){
	write<short>(1);
	write<short>(0);
	write<std::string>(username);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::username_result(bool success, int id){
	write<short>(2);
	write<short>(0);
	write<bool>(success);
	write<int>(id);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::number_of_players_online(int num_online){
	write<short>(3);
	write<short>(0);
	write<short>(num_online);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

//in the omok join lobby, show all the omok game rooms on the server
void Packet::omok_room_info(int id, std::string name, bool is_full){
	write<short>(4);
	write<short>(0);	//determine the size later
	write<int>(id);
	write<std::string>(name);
	write<bool>(is_full);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_room_update(int id, char action, bool is_full){
	write<short>(18);
	write<short>(0);
	write<int>(id);
	//1 = change number of users in omok room
	//2 = remove omok room
	write<char>(action);
	write<bool>(is_full);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_join_room(int room_id){
	write<short>(5);
	write<short>(0);
	write<int>(room_id);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_room_unavailable(){
	write<short>(20);
	write<short>(0);
}

void Packet::omok_create_room(std::string room_name){
	write<short>(6);
	write<short>(0);
	write<std::string>(room_name);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

//puts a player into omok room
void Packet::omok_room_admittance(OmokGame *game){	//another player is the leader
	write<short>(7);
	write<short>(0);	//determine the size later
	write<int>(game->get_id());
	write<std::string>(game->get_name());
	write<int>(game->get_leader()->get_id());
	write<std::string>(game->get_leader()->get_name());
	
	//write the size, minus the header
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_room_create(OmokGame *game){		//the player receiving this packet is the leader
	write<short>(8);
	write<short>(0);	//determine the size later
	write<int>(game->get_id());
	write<std::string>(game->get_name());
	
	//write the size, minus the header
	*(short*)(buffer_ + 2) = length_ - 4;
}

//within the omok game
void Packet::omok_start_game(){
	write<short>(9);
	write<short>(0);
}

void Packet::omok_place_piece(int x, int y){
	write<short>(10);
	write<short>(0);
	write<char>(x);
	write<char>(y);
	
	//now write the size
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_win_game(){
	write<short>(11);
	write<short>(0);
}

void Packet::omok_lose_game(){
	write<short>(12);
	write<short>(0);
}

void Packet::omok_message(std::string player_name, std::string message){
	write<short>(13);
	write<short>(0);	//determine the size later
	write<std::string>(player_name);
	write<std::string>(message);
	
	//write the size, minus the header
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_player_has_joined(Player *player){
	write<short>(14);
	write<short>(0);	//determine the size later
	write<int>(player->get_id());
	write<std::string>(player->get_name());
	
	//write the size, minus the header
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_player_has_left(Player *player){
	write<short>(15);
	write<short>(0);
	write<int>(player->get_id());
	write<std::string>(player->get_name());
	
	*(short*)(buffer_ + 2) = length_ - 4;
}

void Packet::omok_switch_turn(){
	write<short>(16);
	write<short>(0);
}

void Packet::omok_room_closure(){
	write<short>(17);
	write<short>(0);
}

void Packet::omok_room_leave(){
	write<short>(19);
	write<short>(0);
}

char *Packet::get(){
	return buffer_;
}

size_t Packet::size(){
	return length_;
}