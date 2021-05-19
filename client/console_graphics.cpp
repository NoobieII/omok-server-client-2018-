#include "console_graphics.hpp"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

#include <fstream>

static int frames_per_sec = 3;
static unsigned long CLOCKS_PER_FRAME = CLOCKS_PER_SEC / frames_per_sec;
static int screen_width = 80;
static int screen_height = 50;
static char background = ' ';

static char* screen = new char[screen_width * screen_height + 1]();
static char* last_screen = new char[screen_width * screen_height + 1]();

static const Sprite null_sprite(0, 1, 1, "\0");

void set_framerate(int n){
	frames_per_sec = n;
	CLOCKS_PER_FRAME = CLOCKS_PER_SEC / frames_per_sec;
}

void set_screen_dimensions(int width, int height){
	if(screen_width < 1 || screen_height < 1 || screen_width > 100 || screen_height > 100){
		return;
	}
	else{
		delete[] screen;
		delete[] last_screen;
		screen_width = width;
		screen_height = height;
		screen = new char[screen_width * screen_height + 1];
		last_screen = new char[screen_width * screen_height + 1];
		
		memset(screen, background, screen_width * screen_height);
		memset(last_screen, background, screen_width * screen_height);
		screen[screen_width * screen_height] = '\0';
		screen[screen_width * screen_height] = '\0';
	}
}

void set_background(int val){
	background = val;
}

void render(void){
	/*if there is a null character in the screen string for console display
	 *(when char* screen is initialized and not rendered yet, at the start of
	 *the application or in the case of resizing the screen
	 */
	if(strlen(screen) < screen_width * screen_height){
		for(int i = 0; i < screen_width * screen_height; ++i){
			if(screen[i] == 0){
				screen[i] = background;
			}
		}
	}
	/*make the right ends of the screen strings appended with a newline
	 */
	for(int row = 1; row < screen_height; ++row){
		screen[row * screen_width - 1] = '\n';
	}
	
	screen[screen_height * screen_width - 1] = '\0';
	
	printf("\n\r%s", screen);
	
	/*remove what was drawn on screen/fill with background character
	 */
	memcpy(last_screen, screen, screen_width * screen_height + 1);
	
	for(int i = 0; i < screen_width * screen_height; ++i){
		screen[i] = background;
	}
}

void draw_text(const char* str, int x, int y, int width, int height, int frame){
	
	/*if image/string is out of bounds*/
	if(x + width < 1 || x > screen_width - 1 || y + height < 1 || y > screen_height - 1){
		return;
	}
	
	for(int row = 0; row < height; ++row){
		for(int col = 0; col < width; ++col){
			char ch = str[frame * height * width + row * width + col];
			if(ch && (col + x) > -1 && (col + x) < screen_width && (row + y) > -1 && (row + y) < screen_height){
				screen[(row + y) * screen_width + (col + x)] = ch;
			}
		}
	}
}

//draw a string onto the screen, following format that a '\n' moves down one line, '\r'
//moves to the left of the line, '\t' moves forward four characters.
void draw_str(const char* str, int x, int y, int width, int height, int length){
	int i = 0;
	int char_num = 0;
	
	while(str[char_num] && ((y + i/width) * screen_width + x + i%width%screen_width) < screen_width * screen_height 
			&& char_num < length && i/width < height){
		switch(str[char_num]){
		case '\n':
			//i is moved down one line, but not moved back. Use "\n\r" when the
			//carriage return is intended.
			i += width;
			break;
		case '\r':
			//i is moved to the start of the line.
			//divide i by the width of each rows and multiply by width again.
			//i gets any remainder truncated in the division since it is an int.
			i = i/width * width;
			break;
		case '\t':
			//i is moved to the next 4 characters on the line.
			//first move i to the start of the line.
			//then calculate which group of 4 characters i is in and move it to
			//the start of that group. Then add 4 to i to move to the start of next group.
			i = i/width * width + (i%width)/4 * 4 + 4;
			break;
		default:
			screen[(y + i/width) * screen_width + x + i%width%screen_width] = str[char_num];
			i++;
			break;
		}
		char_num++;
	}
}
	

void draw_str_wrapped(const char* str, int x, int y, int width, int height){
	int str_index = 0;
	char next_word[128];
	
	/*if image/string is out of bounds*/
	if(x + width < 1 || x > screen_width - 1 || y + height < 1 || y > screen_height - 1){
		return;
	}
	
	
	for(int row = 0; row < height; ++row){
		for(int col = 0; col < width; ++col){
			
			/*to skip the first space in the line*/
			if(col == 0 && str[str_index] == ' '){
				str_index++;
			}
			
			/*if character of string not a null value*/
			if(str[str_index]){
				
				
				/*this part is for word wrapping or checking for a new line*/
				sscanf(&str[str_index], "%s", next_word);
				
				if(str[str_index] == '\n'){
					row++;
					str_index++;
					
					if(row >= height){
						break;
					}
				}
				else if(str[str_index] == '\r'){
					col = -1;
					str_index++;
				}
				else if(str[str_index] == '\t'){
					/*moves the text four spaces since col gets incremented again before next loop*/
					col += 3;
					str_index++;
					
					if(col >= width){
						break;
					}
				}
				else if(str[str_index] != ' ' && strlen(next_word) > (width - col)){
					while(str[str_index] == ' '){
						str_index++;
					}
					break;
				}
				else{
					if(x + col > -1 && x + col < screen_width && y + row > -1 && y + row < screen_height){
						screen[(row + y) * screen_width + (col + x)] = str[str_index];
					}
					str_index++;
				}
			}
			else{
				return;
			}
		}
	}
}

void draw_int_left_end(int num, int x, int y){
	char sz_num[15];
	sprintf(sz_num, "%d", num);
	
	for(int i = 0; sz_num[i]; ++i){
		if((x + i) >= 0 && (x + i) < screen_width && y >= 0 && y < screen_height){
			screen[y * screen_width + x + i] = sz_num[i];
		}
	}
}

void draw_int_right_end(int num, int x, int y){
	char sz_num[15];
	sprintf(sz_num, "%d", num);
	
	for(int i = 0; sz_num[i]; ++i){
		if((x - i) >= 0 && (x - i) < screen_width && y >= 0 && y < screen_height){
			screen[y * screen_width + x - i] = sz_num[strlen(sz_num) - i - 1];
		}
	}
}

void draw_area(int val, int x, int y, int width, int height){
	/*ensure area drawn is within bounds*/
	if(x + width < 1 || x > screen_width - 1 || y + height < 1 || y > screen_height - 1){
		return;
	}
	
	int start_x = x;
	int end_x = x + width;
	int start_y = y;
	int end_y = y + height;
	
	if(start_x < 0){
		start_x = 0;
	}
	if(end_x > screen_width){
		end_x = screen_width;
	}
	int width_x = end_x - start_x;
	if(start_y < 0){
		start_y = 0;
	}
	if(end_y > screen_height){
		end_y = screen_height;
	}
	int width_y = end_y - start_y;
	
	for(int row = 0; row < width_y; ++row){
		for(int col = 0; col < width_x; ++col){
			screen[(row + start_y) * screen_width + (col + start_x)] = val;
		}
	}
}

void draw_sqr(int val, int x, int y, int width, int height){
	/*draw the top side of the square*/
	if(y > -1 && y < screen_height){
		int start_x = x;
		if(start_x > (screen_width - 1)){
			return;
		}
		int end_x = x + width - 1;
		if(start_x < 0){
			start_x = 0;
		}
		if(end_x > screen_width){
			end_x = screen_width;
		}
		for(int i = start_x; i < end_x; ++i){
			screen[y * screen_width + i] = val;
		}
	}
	/*draw the left side of the square*/
	if(x > -1 && x < screen_width){
		int start_y = y;
		if(start_y > (screen_height - 1)){
			return;
		}
		int end_y = y + height;
		if(start_y < 0){
			start_y = 0;
		}
		if(end_y > screen_height){
			end_y = screen_height;
		}
		for(int i = start_y; i < end_y; ++i){
			screen[i * screen_width + x] = val;
		}
	}
	
	/*draw the right side of the square*/
	if((x + width) > 0 && (x + width - 1) < screen_width){
		int start_y = y;
		int end_y = y + height;
		if(start_y < 0){
			start_y = 0;
		}
		if(end_y > screen_height){
			end_y = screen_height;
		}
		for(int i = start_y; i < end_y; ++i){
			screen[i * screen_width + x + width - 1] = val;
		}
	}
	
	/*draw the bottom side of the square*/
	if((y + height) > 0 && (y + height - 1) < screen_height){
		int start_x = x;
		int end_x = x + width;
		if(start_x < 0){
			start_x = 0;
		}
		if(end_x > screen_width){
			end_x = screen_width;
		}
		for(int i = start_x; i < end_x; ++i){
			screen[(y + height - 1) * screen_width + i] = val;
		}
	}
}

void draw_line(int val, int x, int y, int x2, int y2){
	if((x < 0 && x2 < 0) || (y < 0 && y2 < 0) || (x >= screen_width && x2 >= screen_width) ||
		(y >= screen_height && y2 >= screen_height) || (val < 32)){
		return;
	}
	
	/*if slope is infinite*/
	if(x2 - x == 0){
		if(y > y2){
			int tmp = y2;
			y2 = y;
			y = tmp;
		}
		
		for(int i = y; i <= y2; ++i){
			if(i >= 0 && i < screen_height && x >= 0 && x < screen_width){
				screen[i * screen_width + x] = val;
			}
		}
		return;
	}
	
	float rise_over_run = ((float)(y2 - y) / (float)(x2 - x));
	float run_over_rise = ((float)(x2 - x) / (float)(y2 - y));
		
	/*if the line slope <= 1*/
	if(abs(x2 - x) >= abs(y2 - y)){
		if(x > x2){
			int tmp = x2;
			x2 = x;
			x = tmp;
		}
		
		for(int i = x; i <= x2; ++i){
			/*calculate y value once, 0.5 is added to round to nearest integer when casted back*/
			int y_value = y + ((float)(i - x) * rise_over_run + 0.5);
			if(i >= 0 && i < screen_width && y_value >= 0 && y_value < screen_height){
				screen[y_value * screen_width + i] = val;
			}
		}
		return;
	}
	else{
		if(y > y2){
			int tmp = y2;
			y2 = y;
			y = tmp;
		}
		
		for(int i = y; i <= y2; ++i){
			int x_value = x + ((float)(i - y) * run_over_rise + 0.5);
			if(i >= 0 && i < screen_height && x_value >= 0 && x_value < screen_width){
				screen[i * screen_width + x_value] = val;
			}
		}
		return;
	}
}

void draw_image(Sprite* s, int x, int y, int frame){
	const char* sprite = s->get_sprite(frame);
	int image_width = s->get_width();
	int image_height = s->get_height();

	/*if image is out of bounds from screen*/
	if(x + image_width <= 0 || x >= screen_width || y + image_height <= 0 || y >= screen_height) return;
	
	for(int row = 0; row < image_height; ++row){
		for(int col = 0; col < image_width; ++col){
			char ch = sprite[row * image_width + col];
			if(ch && (col + x) > -1 && (col + x) < screen_width && (row + y) > -1 && (row + y) < screen_height){
				screen[(row + y) * screen_width + (col + x)] = ch;
			}
		}
	}
}

void draw_image_clip(Sprite* s, int x, int y, int x_from_sprite, int y_from_sprite, int width, int height, int frame){
	const char* sprite = s->get_sprite(frame);
	
	//resize the selection of image to be cropped
	if(x_from_sprite < 0){
		width = x_from_sprite + width;
		x_from_sprite = 0;
	}
	if(x_from_sprite >= s->get_width()){
		return;
	}
	if(y_from_sprite < 0){
		height = y_from_sprite + height;
		y_from_sprite = 0;
	}
	if(y_from_sprite >= s->get_height()){
		return;
	}
	
	//if width extends beyond the sprite
	if(x_from_sprite + width > s->get_width()){
		width = s->get_width() - x_from_sprite;
	}
	if(y_from_sprite + height > s->get_height()){
		height = s->get_height() - y_from_sprite;
	}
	
	
	for(int row = 0; row < height; ++row){
		for(int col = 0; col < width; ++col){
			char ch = sprite[(row + y_from_sprite) * s->get_width() + (col + x_from_sprite)];
			if(ch && (col + x) > -1 && (col + x) < screen_width && (row + y) > -1 && (row + y) < screen_height){
				screen[(row + y) * screen_width + (col + x)] = ch;
			}
		}
	}
}


void draw_layer(ScreenLayer* s, int x, int y){
	const char* sprite = s->get_screen();
	int image_width = s->get_width();
	int image_height = s->get_height();

	/*if image is out of bounds from screen*/
	if(x + image_width <= 0 || x >= screen_width || y + image_height <= 0 || y >= screen_height) return;
	
	for(int row = 0; row < image_height; ++row){
		for(int col = 0; col < image_width; ++col){
			char ch = sprite[row * image_width + col];
			if(ch && (col + x) > -1 && (col + x) < screen_width && (row + y) > -1 && (row + y) < screen_height){
				screen[(row + y) * screen_width + (col + x)] = ch;
			}
		}
	}
}

int current_frame(){
	return clock() / CLOCKS_PER_FRAME;
}

ScreenLayer::ScreenLayer(int width, int height):
screen_width_(width),
screen_height_(height)
{
	screen_ = new char[screen_width_ * screen_height_]();
}

ScreenLayer::~ScreenLayer(){
	if(screen_){
		delete[] screen_;
	}
}

void ScreenLayer::draw_text(const char* str, int x, int y, int width, int height, int frame){
	
	/*if image/string is out of bounds*/
	if(x + width < 1 || x > screen_width_ - 1 || y + height < 1 || y > screen_height_ - 1){
		return;
	}
	
	for(int row = 0; row < height; ++row){
		for(int col = 0; col < width; ++col){
			char ch = str[frame * height * width + row * width + col];
			if(ch && (col + x) > -1 && (col + x) < screen_width_ && (row + y) > -1 && (row + y) < screen_height_){
				screen_[(row + y) * screen_width_ + (col + x)] = ch;
			}
		}
	}
}

void ScreenLayer::draw_str(const char* str, int x, int y, int width, int height, int length){
	int i = 0;
	int char_num = 0;
	
	while(str[char_num] && ((y + i/width) * screen_width_ + x + i%width%screen_width_) < screen_width_ * screen_height_ 
			&& char_num < length && i/width < height){
		switch(str[char_num]){
		case '\n':
			//i is moved down one line, but not moved back. Use "\n\r" when the
			//carriage return is intended.
			i += width;
			break;
		case '\r':
			//i is moved to the start of the line.
			//divide i by the width of each rows and multiply by width again.
			//i gets any remainder truncated in the division since it is an int.
			i = i/width * width;
			break;
		case '\t':
			//i is moved to the next 4 characters on the line.
			//first move i to the start of the line.
			//then calculate which group of 4 characters i is in and move it to
			//the start of that group. Then add 4 to i to move to the start of next group.
			i = i/width * width + (i%width)/4 * 4 + 4;
			break;
		default:
			screen_[(y + i/width) * screen_width_ + x + i%width%screen_width_] = str[char_num];
			i++;
			break;
		}
		char_num++;
	}
}

void ScreenLayer::draw_str_wrapped(const char* str, int x, int y, int width, int height){
	int str_index = 0;
	char next_word[128];
	
	/*if image/string is out of bounds*/
	if(x + width < 1 || x > screen_width_ - 1 || y + height < 1 || y > screen_height_ - 1){
		return;
	}
	
	
	for(int row = 0; row < height; ++row){
		for(int col = 0; col < width; ++col){
			
			/*to skip the first space in the line*/
			if(col == 0 && str[str_index] == ' '){
				str_index++;
			}
			
			/*if character of string not a null value*/
			if(str[str_index]){
				
				
				/*this part is for word wrapping or checking for a new line*/
				sscanf(&str[str_index], "%s", next_word);
				
				if(str[str_index] == '\n'){
					row++;
					str_index++;
					
					if(row >= height){
						break;
					}
				}
				else if(str[str_index] == '\r'){
					col = -1;
					str_index++;
				}
				else if(str[str_index] == '\t'){
					/*moves the text four spaces since col gets incremented again before next loop*/
					col += 3;
					str_index++;
					
					if(col >= width){
						break;
					}
				}
				else if(str[str_index] != ' ' && strlen(next_word) > (width - col)){
					while(str[str_index] == ' '){
						str_index++;
					}
					break;
				}
				else{
					if(x + col > -1 && x + col < screen_width_ && y + row > -1 && y + row < screen_height_){
						screen_[(row + y) * screen_width_ + (col + x)] = str[str_index];
					}
					str_index++;
				}
			}
			else{
				return;
			}
		}
	}
}

void ScreenLayer::draw_area(int val, int x, int y, int width, int height){
	
	/*ensure area drawn is within bounds*/
	if(x + width < 1 || x > screen_width_ - 1 || y + height < 1 || y > screen_height_ - 1){
		return;
	}
	
	int start_x = x;
	int end_x = x + width;
	int start_y = y;
	int end_y = y + height;
	
	if(start_x < 0){
		start_x = 0;
	}
	if(end_x > screen_width_){
		end_x = screen_width_;
	}
	int width_x = end_x - start_x;
	if(start_y < 0){
		start_y = 0;
	}
	if(end_y > screen_height_){
		end_y = screen_height_;
	}
	int width_y = end_y - start_y;
	
	for(int row = 0; row < width_y; ++row){
		for(int col = 0; col < width_x; ++col){
			screen_[(row + start_y) * screen_width_ + (col + start_x)] = val;
		}
	}
}

void ScreenLayer::draw_sqr(int val, int x, int y, int width, int height){
	/*draw the top side of the square*/
	if(y > -1 && y < screen_height_){
		int start_x = x;
		if(start_x > (screen_width_ - 1)){
			return;
		}
		int end_x = x + width - 1;
		if(start_x < 0){
			start_x = 0;
		}
		if(end_x > screen_width_){
			end_x = screen_width_;
		}
		for(int i = start_x; i < end_x; ++i){
			screen_[y * screen_width_ + i] = val;
		}
	}
	/*draw the left side of the square*/
	if(x > -1 && x < screen_width_){
		int start_y = y;
		if(start_y > (screen_height_ - 1)){
			return;
		}
		int end_y = y + height;
		if(start_y < 0){
			start_y = 0;
		}
		if(end_y > screen_height_){
			end_y = screen_height_;
		}
		for(int i = start_y; i < end_y; ++i){
			screen_[i * screen_width_ + x] = val;
		}
	}
	
	/*draw the right side of the square*/
	if((x + width) > 0 && (x + width - 1) < screen_width_){
		int start_y = y;
		int end_y = y + height;
		if(start_y < 0){
			start_y = 0;
		}
		if(end_y > screen_height_){
			end_y = screen_height_;
		}
		for(int i = start_y; i < end_y; ++i){
			screen_[i * screen_width_ + x + width - 1] = val;
		}
	}
	
	/*draw the bottom side of the square*/
	if((y + height) > 0 && (y + height - 1) < screen_height_){
		int start_x = x;
		int end_x = x + width;
		if(start_x < 0){
			start_x = 0;
		}
		if(end_x > screen_width_){
			end_x = screen_width_;
		}
		for(int i = start_x; i < end_x; ++i){
			screen_[(y + height - 1) * screen_width_ + i] = val;
		}
	}
}

void ScreenLayer::draw_line(int val, int x, int y, int x2, int y2){
	if((x < 0 && x2 < 0) || (y < 0 && y2 < 0) || (x >= screen_width_ && x2 >= screen_width_) ||
		(y >= screen_height_ && y2 >= screen_height_) || (val < 32)){
		return;
	}
	
	/*if slope is infinite*/
	if(x2 - x == 0){
		if(y > y2){
			int tmp = y2;
			y2 = y;
			y = tmp;
		}
		
		for(int i = y; i <= y2; ++i){
			if(i >= 0 && i < screen_height_ && x >= 0 && x < screen_width_){
				screen_[i * screen_width_ + x] = val;
			}
		}
		return;
	}
	
	float rise_over_run = ((float)(y2 - y) / (float)(x2 - x));
	float run_over_rise = ((float)(x2 - x) / (float)(y2 - y));
		
	/*if the line slope <= 1*/
	if(abs(x2 - x) >= abs(y2 - y)){
		if(x > x2){
			int tmp = x2;
			x2 = x;
			x = tmp;
		}
		
		for(int i = x; i <= x2; ++i){
			/*calculate y value once, 0.5 is added to round to nearest integer when casted back*/
			int y_value = y + ((float)(i - x) * rise_over_run + 0.5);
			if(i >= 0 && i < screen_width_ && y_value >= 0 && y_value < screen_height_){
				screen_[y_value * screen_width_ + i] = val;
			}
		}
		return;
	}
	else{
		if(y > y2){
			int tmp = y2;
			y2 = y;
			y = tmp;
		}
		
		for(int i = y; i <= y2; ++i){
			int x_value = x + ((float)(i - y) * run_over_rise + 0.5);
			if(i >= 0 && i < screen_height_ && x_value >= 0 && x_value < screen_width_){
				screen_[i * screen_width_ + x_value] = val;
			}
		}
		return;
	}
}

// screen screen_width screen_height
void ScreenLayer::draw_image(Sprite* s, int x, int y, int frame){
	const char* sprite = s->get_sprite(frame);
	int image_width = s->get_width();
	int image_height = s->get_height();

	/*if image is out of bounds from screen*/
	if(x + image_width <= 0 || x >= screen_width_ || y + image_height <= 0 || y >= screen_height_) return;
	
	for(int row = 0; row < image_height; ++row){
		for(int col = 0; col < image_width; ++col){
			char ch = sprite[frame * image_height * image_width + row * image_width + col];
			if(ch && (col + x) > -1 && (col + x) < screen_width_ && (row + y) > -1 && (row + y) < screen_height_){
				screen_[(row + y) * screen_width_ + (col + x)] = ch;
			}
		}
	}
}

void ScreenLayer::draw_image_clip(Sprite* s, int x, int y, int x_from_sprite, int y_from_sprite, int width, int height, int frame){
	const char* sprite = s->get_sprite(frame);
	
	//resize the selection of image to be cropped
	if(x_from_sprite < 0){
		width = x_from_sprite + width;
		x_from_sprite = 0;
	}
	if(x_from_sprite >= s->get_width()){
		return;
	}
	if(y_from_sprite < 0){
		height = y_from_sprite + height;
		y_from_sprite = 0;
	}
	if(y_from_sprite >= s->get_height()){
		return;
	}
	
	//if width extends beyond the sprite
	if(x_from_sprite + width > s->get_width()){
		width = s->get_width() - x_from_sprite;
	}
	if(y_from_sprite + height > s->get_height()){
		height = s->get_height() - y_from_sprite;
	}
	
	
	for(int row = 0; row < height; ++row){
		for(int col = 0; col < width; ++col){
			char ch = sprite[(row + y_from_sprite) * s->get_width() + (col + x_from_sprite)];
			if(ch && (col + x) > -1 && (col + x) < screen_width_ && (row + y) > -1 && (row + y) < screen_height_){
				screen_[(row + y) * screen_width_ + (col + x)] = ch;
			}
		}
	}
}

void ScreenLayer::draw_int_left_end(int num, int x, int y){
	char sz_num[15];
	sprintf(sz_num, "%d", num);
	
	for(int i = 0; sz_num[i]; ++i){
		if((x + i) >= 0 && (x + i) < screen_width_ && y >= 0 && y < screen_height_){
			screen_[y * screen_width_ + x + i] = sz_num[i];
		}
	}
}

void ScreenLayer::draw_int_right_end(int num, int x, int y){
	char sz_num[15];
	sprintf(sz_num, "%d", num);
	
	for(int i = 0; sz_num[i]; ++i){
		if((x - i) >= 0 && (x - i) < screen_width_ && y >= 0 && y < screen_height_){
			screen_[y * screen_width_ + x - i] = sz_num[strlen(sz_num) - i - 1];
		}
	}
}

void ScreenLayer::clear(){
	memset(screen_, 0, screen_width_ * screen_height_);
}

int ScreenLayer::get_width(){
	return screen_width_;
}

int ScreenLayer::get_height(){
	return screen_height_;
}

const char* ScreenLayer::get_screen(){
	return screen_;
}


Sprite ScreenLayer::get_image_clip(int x, int y, int width, int height){
	if(width < 1 || height < 1){
		return null_sprite;
	}
	
	//must use static array, not dynamic. Array can't be deallocated after sprite is created
	static char buf[10000];
	memset(buf, 0, width * height);
	
	//copy the contents of the screen line by line to the sprite
	for(int i = 0; i < height; ++i){
		if(i + y < 0 || i + y >= screen_height_){
			continue;
		}
		
		if(x <= 0 && x + width <= screen_width_ && x + width > 0){
			memcpy(buf + i * width - x, screen_ + (i + y) * screen_width_, x + width);
		}
		else if(x <= 0 && x + width > screen_width_){
			memcpy(buf + i * width - x, screen_ + (i + y) * screen_width_, screen_width_);
		}
		else if(x > 0 && x + width <= screen_width_ && x + width > 0){
			memcpy(buf + i * width, screen_ + (i + y) * screen_width_ + x, width);
		}
		else if(x > 0 && x + width > screen_width_){
			memcpy(buf + i * width, screen_ + (i + y) * screen_width_ + x, screen_width_ - x);
		}
	}
	
	//must return using constructor/copy constructor, so 
	return Sprite(0, width, height, buf, false, false, 1, 1);
}
		
Sprite::Sprite():
id_(0),
width_(0),
height_(0),
is_animation_(0),
is_loop_(0),
frame_duration_(0),
animation_size_(0),
sprite_(0)
{
}

Sprite::Sprite(int id, int width, int height, const char* sprite, bool is_animation,
bool is_loop, int frame_duration, int animation_size):
id_(id),
width_(width),
height_(height),
is_animation_(is_animation),
is_loop_(is_loop),
frame_duration_(frame_duration),
animation_size_(animation_size),
sprite_(new unsigned char[width * height * animation_size])
{
	if(sprite){
		memcpy(sprite_, sprite, width * height * animation_size);
	}
	else{
		memset(sprite_, 0, width * height * animation_size);
	}
}

Sprite::Sprite(const Sprite &s):
id_(s.id_),
width_(s.width_),
height_(s.height_),
is_animation_(s.is_animation_),
is_loop_(s.is_loop_),
frame_duration_(s.frame_duration_),
animation_size_(s.animation_size_),
sprite_(0)
{
	if(s.sprite_){
		sprite_ = new unsigned char[s.width_ * s.height_ * s.animation_size_];
		memcpy(sprite_, s.sprite_, width_ * height_ * animation_size_);
	}
}

Sprite::~Sprite(){
	if(sprite_) delete[] sprite_;
}

Sprite& Sprite::operator=(const Sprite &s){
	id_ = s.id_;
	width_ = s.width_;
	height_ = s.height_;
	is_animation_ = s.is_animation_;
	is_loop_ = s.is_loop_;
	frame_duration_ = s.frame_duration_;
	animation_size_ = s.animation_size_;
	
	if(sprite_){
		delete[] sprite_;
	}
	sprite_ = 0;
	
	if(s.sprite_){
		sprite_ = new unsigned char[s.width_ * s.height_ * s.animation_size_];
		memcpy(sprite_, s.sprite_, s.width_ * s.height_ * s.animation_size_);
	}
	
	return *this;
}

int Sprite::get_id(){
	return id_;
}

int Sprite::get_width(){
	return width_;
}

int Sprite::get_height(){
	return height_;
}

const char* Sprite::get_sprite(int frame){
	if(is_loop_){
		//return the frame that matches the current time for looping animations
		
		return (const char*)(&sprite_[width_ * height_
		* ((clock() / (CLOCKS_PER_FRAME * frame_duration_)) % animation_size_)]);
	}
	else
	{
		if(frame >= animation_size_){
			return 0;
		}
		return (const char*)(&sprite_[width_ * height_ * frame]);
	}
}

bool Sprite::is_animation(){
	return is_animation_;
}

bool Sprite::is_loop(){
	return is_loop_;
}

int Sprite::get_frame_duration(){
	return frame_duration_;
}

int Sprite::get_animation_size(){
	return animation_size_;
}

std::ifstream& Sprite::read_from_file(std::ifstream& in){
	in.read(reinterpret_cast<char*>(&id_), sizeof(int));
	if(in.eof()){
		return in;
	}
	
	if(sprite_){
		delete[] sprite_;
	}
	sprite_ = 0;
	
	in.read(reinterpret_cast<char*>(&width_), sizeof(int));
	in.read(reinterpret_cast<char*>(&height_), sizeof(int));
	in.read(reinterpret_cast<char*>(&is_animation_), sizeof(bool));
	in.read(reinterpret_cast<char*>(&is_loop_), sizeof(bool));
	in.read(reinterpret_cast<char*>(&frame_duration_), sizeof(int));
	in.read(reinterpret_cast<char*>(&animation_size_), sizeof(int));
	sprite_ = new unsigned char[width_ * height_ * animation_size_];
	in.read((char*)(sprite_), width_ * height_ * animation_size_);
	return in;
}

void Sprite::read_from_file(const char* filename){
	std::ifstream in;
	in.open(filename, std::ios::binary);
	if(in.fail()){
		in.close();
		return;
	}
	
	if(sprite_){
		delete[] sprite_;
	}
	sprite_ = 0;
	
	in.read(reinterpret_cast<char*>(&id_), sizeof(int));
	in.read(reinterpret_cast<char*>(&width_), sizeof(int));
	in.read(reinterpret_cast<char*>(&height_), sizeof(int));
	in.read(reinterpret_cast<char*>(&is_animation_), sizeof(bool));
	in.read(reinterpret_cast<char*>(&is_loop_), sizeof(bool));
	in.read(reinterpret_cast<char*>(&frame_duration_), sizeof(int));
	in.read(reinterpret_cast<char*>(&animation_size_), sizeof(int));
	sprite_ = new unsigned char[width_ * height_ * animation_size_];
	in.read((char*)(sprite_), width_ * height_ * animation_size_);
	
	in.close();
}

std::ofstream& Sprite::write_to_file(std::ofstream& out){
	out.write(reinterpret_cast<char*>(&id_), sizeof(int));
	out.write(reinterpret_cast<char*>(&width_), sizeof(int));
	out.write(reinterpret_cast<char*>(&height_), sizeof(int));
	out.write(reinterpret_cast<char*>(&is_animation_), sizeof(bool));
	out.write(reinterpret_cast<char*>(&is_loop_), sizeof(bool));
	out.write(reinterpret_cast<char*>(&frame_duration_), sizeof(int));
	out.write(reinterpret_cast<char*>(&animation_size_), sizeof(int));
	out.write(reinterpret_cast<char*>(sprite_), width_ * height_ * animation_size_);
	return out;
}