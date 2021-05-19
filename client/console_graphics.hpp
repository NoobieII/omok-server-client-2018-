#ifndef CONSOLE_GRAPHICS_H
#define CONSOLE_GRAPHICS_H

#include <iosfwd>

class ScreenLayer;
class Sprite;

void set_framerate(int n);
/*this sets the dimensions of the screen to print.
 */
void set_screen_dimensions(int width, int height);
/*set the background character of the screen (by default it is ' ').
 *any section of the screen that has not been drawn on will have the background character.
 */
void set_background(int val);
/*print what was drawn by the bottom drawing functions
 */
void render(void);
/*Draw raw text onto the screen and treat str like the characters of a sprite.
  x and y is the location of where to draw on screen, width and height are the
  dimensions and frame is interpreted as where in str to output onto the string
  if str is stored in this particular manner: char str[frame * width * height];
 */
void draw_text(const char* str, int x, int y, int width, int height, int frame = 0);
//draw_str draws a string on coordinates x, y on the screen at the maximum width and height of the
//field defined. length is the maximum number of string characters to draw. If the string terminates
//before str[length], then the function will automatically stop drawing the string.
//characters '\n', '\r', and '\t' can be used in the string to move down one line, move to
//the start of the line, and move to the start of the next 4 characters, respectively.
void draw_str(const char* str, int x, int y, int width = 100, int height = 1, int length = 10000);
void draw_str_wrapped(const char* str, int x, int y, int width = 100, int height = 100);
void draw_area(int val, int x, int y, int width, int height);
/*Purely for convenience, just draw a square on the screen with the character value val
 */
void draw_sqr(int val, int x, int y, int width, int height);
/*draw a line on screen from (x,y) to (x2, y2)
 */
void draw_line(int val, int x, int y, int x2, int y2);
void draw_image(Sprite* s, int x, int y, int frame = 0);
void draw_image_clip(Sprite* s, int x, int y, int x_from_sprite, int y_from_sprite, int width, int height, int frame = 0);
void draw_int_left_end(int num, int x, int y);
void draw_int_right_end(int num, int x, int y);
void draw_layer(ScreenLayer* s, int x, int y);

int current_frame();
const char* last_render();

class ScreenLayer{
public:
	ScreenLayer(int width, int height);
	~ScreenLayer();
	
	void draw_text(const char* str, int x, int y, int width, int height, int frame = 0);
	void draw_str(const char* str, int x, int y, int width = 100, int height = 1, int length = 10000);
	void draw_str_wrapped(const char* str, int x, int y, int width = 100, int height = 100);
	void draw_area(int val, int x, int y, int width, int height);
	void draw_sqr(int val, int x, int y, int width, int height);
	void draw_line(int val, int x, int y, int x2, int y2);
	void draw_image_clip(Sprite* s, int x, int y, int x_from_sprite, int y_from_sprite, int width, int height, int frame = 0);
	void draw_image(Sprite* s, int x, int y, int frame = 0);
	void draw_int_left_end(int num, int x, int y);
	void draw_int_right_end(int num, int x, int y);
	void clear();
	
	int get_width();
	int get_height();
	const char* get_screen();
	
	//March 16, 2018, 9.08
	//a function that returns a sprite instance of a section of the layer.
	Sprite get_image_clip(int x, int y, int width, int height);
	
private:
	int screen_width_;
	int screen_height_;
	char* screen_;
};

class Sprite{
public:
	Sprite();
	Sprite(int id, int width, int height, const char* sprite = 0, bool is_animation = false,
			bool is_loop = false, int frame_duration = 1, int animation_size = 1);
	
	Sprite(const Sprite &s);
	~Sprite();
	
	Sprite& operator=(const Sprite &s);
	
	int get_id();
	int get_width();
	int get_height();
	const char* get_sprite(int frame = 0);
	bool is_animation();
	bool is_loop();
	int get_frame_duration();
	int get_animation_size();
	
	std::ifstream& read_from_file(std::ifstream& in);
	void read_from_file(const char* filename);
	std::ofstream& write_to_file(std::ofstream& out);
private:
	int id_;
	int width_;
	int height_;
	bool is_animation_;
	bool is_loop_;
	int frame_duration_;
	int animation_size_;
	unsigned char* sprite_;
};

#endif