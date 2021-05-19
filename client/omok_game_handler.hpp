#ifndef OMOK_GAME_HANDLER
#define OMOK_GAME_HANDER

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

struct UIInfo;

//this is to be put in a deadline timer when it
//is the player's turn
void omok_switch_turn_handler(boost::shared_ptr<boost::asio::deadline_timer> t, UIInfo *ui_info);

#endif