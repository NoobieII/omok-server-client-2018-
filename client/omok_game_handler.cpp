#include "interface.hpp"
#include "omok_game.hpp"
#include "omok_game_handler.hpp"
#include "omok_player.hpp"
#include "packet.hpp"
#include "player.hpp"

void omok_switch_turn_handler(boost::shared_ptr<boost::asio::deadline_timer> t, UIInfo *ui_info){
	//check if the user is still in the game room
	if(ui_info->state_ != UI_OMOK_GAME || !ui_info->is_omok_game_in_session_){
		return;
	}
	
	Player *player = ui_info->session_->get_player();
	OmokGame *omok_game = player->get_omok_game();
	
	if(!omok_game){
		return;
	}
	
	//check if the time remaining is zero
	if(omok_game->get_time_remaining() == 0){
		//switch the turns
		omok_game->switch_turn();
		omok_game->set_remaining_time(30);
		
		//tell the other player it is his turn
		Packet packet16;
		packet16.omok_switch_turn();
		player->send(&packet16);
	}
}
		