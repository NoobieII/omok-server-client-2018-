#ifndef OMOK_PLAYER_HPP
#define OMOK_PLAYER_HPP

#include <string>

class OmokPlayer{
public:
	OmokPlayer(int id, std::string name);
	~OmokPlayer();
	
	int get_id();
	std::string get_name();
private:
	int id_;
	std::string name_;
};

#endif