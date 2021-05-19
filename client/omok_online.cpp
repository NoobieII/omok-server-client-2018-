#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>

#include "console_graphics.hpp"
#include "interface.hpp"
#include "key_event.hpp"
#include "session.hpp"

int main(){
	try{
		//set the screen dimensions and framerate
		set_screen_dimensions(80, 30);
		set_framerate(3);
		
		UIInfo info;
		memset(&info, 0, sizeof(UIInfo));
		info.state_ = UI_ENTER_IP_PORT;
		
		boost::asio::io_service io;
		boost::shared_ptr<Session> session(new Session(io, &info));
		info.session_ = session.get();
		info.io_service_ = &io;
		
		int last_render = 0;
		
		for(;;){
			if(kbhit()){
				handle_input(&info);
			}
			io.poll();
			
			//problem seen by Bambo: screen rolling too much, so limit
			//the framerate.
			if(last_render < current_frame()){
				draw_ui(&info);
				last_render = current_frame();
			}
		}
		
	}
	catch(std::exception &e){
		std::cerr << e.what() << "\n";
	}
	
	return 0;
}