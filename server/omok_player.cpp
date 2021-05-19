#include "omok_player.hpp"

OmokPlayer::OmokPlayer(int id, std::string name):
	id_(id),
	name_(name)
{
}

OmokPlayer::~OmokPlayer(){
}

int OmokPlayer::get_id(){
	return id_;
}

std::string OmokPlayer::get_name(){
	return name_;
}