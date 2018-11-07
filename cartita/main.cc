//last modified 20181105
///{ —————INCLUDES————————————————————————————————————————————
#include <gtkmm.h>

//#include <array>
//#include <cerrno>
//#include <climits>
//#include <codecvt>
//#include <cstdio>
//#include <ctime>
//#include <fstream>
#include <iostream>
//#include <iomanip>
//#include <locale>
#include <sstream>
//#include <stdexcept>
#include <vector>
#include <queue>
#include "termcolor.h"

#ifdef __WIN32
	#include <windows.h> //GetModuleFilename
#endif

using namespace std;
using namespace Gtk;
using Glib::ustring;
using Glib::RefPtr;
using Glib::DateTime;

///} —————————————————————————————————————————————————————————
///{ —————DECLARATIONS————————————————————————————————————————
#define NCARDS 52
#define NCOLS   9
#define NROWS  22
#define NSTEPS 16

class CDeck {
  public:
	CDeck(unsigned long int dealno);
	int GetCard(int row, int col);
	int Rank(int card);
	int Suit(int card);
	int Color(int card);
	int Row(int card);
	int Col(int card);
	char RowColToChar(int row, int col);
	char CardToChar(int card);
	tuple<int,int,int> CharToRowColCard(char c);
	int LastOfCol(int col);
	bool Move(string command);
	queue<tuple<int,int,int,int,int,int>>& GetLastMoves();
	string CommandAsString(tuple<int,int,int,int,int,int> command);
	void PrintCard(int card); 
	void Print();
  private:
	int cards_[NROWS][NCOLS];
	unsigned long int dealno_;
	vector<tuple<int,int,int,int,int,int>> commands_;
	queue<tuple<int,int,int,int,int,int>> lastMoves_;
	int undo_;
	void move_card(int srow, int scol, int drow, int dcol, int nmove);
	bool move_undo();
	bool move_redo();
	int max_moves(bool to_empty_column);
	void make_automatic_moves();
};
class CMyDrawingArea : public DrawingArea {
  public:
	CMyDrawingArea();
	void Init(); //to be called when the host Window is finally ready
	void ScaleBoard(double new_scale);
	double GetScale() { return scaleCard_; };
	void DrawBoard(int skip_card = 0);
	bool onTick(int param);
	virtual ~CMyDrawingArea();
  private:
	CDeck* deck_;
	long int gameno_;
	int cards_[NROWS][NCOLS]; //a local copy of CDeck's to store state previous to a move
	Cairo::RefPtr<Cairo::Surface> frame_;
	Cairo::RefPtr<Cairo::Surface> background_;
	RefPtr<Gdk::Pixbuf> imagesOriginalSize_[NCARDS + 1];
	RefPtr<Gdk::Pixbuf> images_[NCARDS + 1];
	Menu menuPopup_;
	int selectedCard_;
	double scaleCard_;
	int wCard_;
	int hCard_;
	tuple<int,int> pos_[NROWS][NCOLS]; //x,y of each cell
	queue<tuple<int,int,int,int,int,int>> pendingMoves_; //animation
	void OnMenuAbout();
	void OnMenuSelect();
	bool on_button_press_event(GdkEventButton* button_event) override;
	bool on_key_press_event(GdkEventKey* event) override;
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
};
class CMyWindow : public Window {
  public:
	CMyWindow();
	virtual ~CMyWindow();
	CMyDrawingArea area_;
};
class CMyApp: public Application {
  public:
	CMyApp();
	static void ApplyCss(Widget& w, const string& state, const string& css_string);
	CMyWindow& GetWindow();
  private:
	CMyWindow* window_; //the only window of this app
	void on_activate() override;
};
///} —————————————————————————————————————————————————————————
///{ —————GLOBALS—————————————————————————————————————————————
//the global application object
RefPtr<CMyApp> g_App;
///} —————————————————————————————————————————————————————————

///{ CDeck
CDeck::CDeck(unsigned long int dealno) : dealno_(dealno) {
	//clear deck
	for (int row = 0; row < NROWS; ++row) {
		for (int col = 0; col < NCOLS; ++col) cards_[row][col] = 0;
	}

	//microsoft game numbering system
	int deck[NCARDS]; //deck of 52 unique cards
	for (int card = 0; card < NCARDS; card++) deck[card] = card;
    int left = NCARDS;	//cards left to be chosen in shuffle
	unsigned long int seed = dealno;
	for (int i = 0; i < NCARDS; i++) {
		seed = (seed * 214013 + 2531011) & 0xffffffff;
		int card = ((seed >> 16) & 0x7fff) % left;
		cards_[i / 8][(i % 8) + 1] = deck[card] + 1; //-> 1..52
		deck[card] = deck[--left];
	}
	
	undo_ = 0;
}
int CDeck::GetCard(int row, int col) {
	if (row >= 0 && col >= 0 && row < NROWS && col < NCOLS) {
		if (col == 0) {
			if (row < 8) return cards_[row][col];
		}
		else {
			return cards_[row][col];
		}
	}
	cout << "ERROR: invalid pair (" << row << ", " << col << ") in CDeck::GetCard" << endl;
	return -1;
}
int CDeck::Rank(int card) { 
	//1..13
	if (card < 1 || card > NCARDS) {
		cout << "ERROR: CDeck::Rank(" << card << ")" << endl; 
		return 0;
	}
	return (card-1)/4 + 1;
};
int CDeck::Suit(int card) {
	//0..4  clubs, diamonds, hearts, spades
	if (card < 1 || card > NCARDS) {
		cout << "ERROR: CDeck::Suit(" << card << ")" << endl; 
		return 0;
	}
	return (card-1)%4; 
};
int CDeck::Color(int card) { 
	//0..1  black,red
	if (card < 1 || card > NCARDS) {
		cout << "ERROR: CDeck::Color(" << card << ")" << endl; 
		return 0;
	}
	return Suit(card)==0 || Suit(card)==3 ? 0 : 1; 
}; 
int CDeck::Row(int card) {
	if (card < 1 || card > NCARDS) {
		cout << "ERROR: CDeck::Row(" << card << ")" << endl; 
		return 0;
	}
	for (int row = 0; row < NROWS; ++row) {
		for (int col = 0; col < NCOLS; ++col) 
			if (cards_[row][col] == card) return row;
	}
	cout << "ERROR: Not found CDeck::Row(" << card << ")" << endl;
	return 0;
}
int CDeck::Col(int card) {
	if (card < 1 || card > NCARDS) {
		cout << "ERROR: CDeck::Col(" << card << ")" << endl; 
		return 0;
	}
	for (int row = 0; row < NROWS; ++row) {
		for (int col = 0; col < NCOLS; ++col) 
			if (cards_[row][col] == card) return col;
	}
	cout << "ERROR: Not found CDeck::Col(" << card << ")" << endl;
	return 0;
}
int CDeck::LastOfCol(int col) {
	if (col == 0) {
		cout << "ERROR: CDeck::LastOfCol(0)" << endl; 
		return 0;
	}
	int row = NROWS - 1; //last row of column
	while (row >= 0 && !cards_[row][col]) --row;
	return row > -1 ? cards_[row][col] : 0; //0 -> empty col
}
tuple<int,int,int> CDeck::CharToRowColCard(char c) {
	int row = -1;
	int col = -1; 
	int card = -1; //invalid values
	if (c >= '1' && c <= '8') {
		col = c - '0';
		card = LastOfCol(col);
		row = card ? Row(card) : 0;
	}
	else if (c >= 'A' && c <= 'H') {
		//freecells: ABCD, home EFGH
		col = 0;
		row = c - 'A'; 
	}
	else cout << "ERROR: CDeck::char_to_row_col(" << c << ")" << endl;
	return make_tuple(row, col, GetCard(row, col));
}
char CDeck::RowColToChar(int row, int col) {
	char c = '\0'; //invalid value
	if (col == 0) { //freecell or home
		if (row >= 0 && row <= 7) c = 'A' + row;
	}
	else if (col >= 1 && col <= 8) { //column
		c = '0' + col;
	}
	if (!c) 
		cout << "ERROR: CDeck::RowColToChar(" << row << ", " << col << ")" << endl;
	return c;
}
char CDeck::CardToChar(int card) {
	char c = '\0'; //invalid value
	int col = Col(card);
	if (col == 0) { //freecell or home
		int row = Row(card);
		if (row >= 0 && row <= 7) c = 'A' + row;
	}
	else if (col >= 1 && col <= 8) { //column
		c = '0' + col;
	}
	if (!c) 
		cout << "ERROR: CDeck::CardToChar(" << card << ")" << endl;
	return c;
}
string CDeck::CommandAsString(tuple<int,int,int,int,int,int> command) {
	auto [card, srow, scol, drow, dcol, nmove] = command;
	string s {RowColToChar(srow, scol)};
	s += RowColToChar(drow, dcol);
	return s;
}
void CDeck::PrintCard(int card) {
	if (card == 0) cout << "    ";
	else {
		const string ranks = "0A23456789TJQK";
		const string suits[4] = {"♣", "♦", "♥", "♠"};
		if (Color(card) == 1) cout << termcolor::red;
		cout << ranks[Rank(card)] << suits[Suit(card)] << "  ";
		cout << termcolor::reset;
	}
}
void CDeck::Print() {
	cout << "Deal #" << dealno_ << endl;
	//print commands stack
	cout << termcolor::blue;
	for (int i = 0; i < commands_.size(); ++i) {
		cout << CommandAsString(commands_[i]) << "  ";
		if ((i+1) % 8 == 0) cout << endl;
	}
	cout << termcolor::reset;
	cout << endl;

	//print top row: freecells and home
	for (int row = 0; row < 8; ++row) PrintCard(GetCard(row, 0));
	cout << "\n______________________________\n";

	//find longest column
	int max = 0;
	for (int row = 0; row < NROWS; ++row) {
		for (int col = 1; col < NCOLS; ++col) {
			if (cards_[row][col] > 0 && row > max) max = row;
		}
	}
	++max;

	//for (int row = 0; row < NROWS; ++row) {
	for (int row = 0; row < max; ++row) {
		for (int col = 1; col < NCOLS; ++col) PrintCard(cards_[row][col]);
		cout << endl;
	}
	cout << "\n==============================" << endl;;
}
int CDeck::max_moves(bool to_empty_column) {
	///{ calculation
	//f: freecells 	c: free columns  	m: moves allowed
	//to non-empty column:			to empty column:
	//	f	c	m						f	c	m
	//---------------					---------------
	//	0	0	1						0	1	1
	//	1	0	2						1	1	2
	//	2	0	3						2	1	3
	// 	3	0	4						3	1	4
	//	4	0	5						4	1	5
	// 	0	1	2						0	2	2
	//	1	1	4						1	2	4
	//	2	1	6						2	2	6
	//	3	1	8						etc...
	//	4	1	10
	//	0	2	4
	//	1	2	8
	//	2	2	12 
	//	0	3	8 (same as 1 2)
	//	etc...
	
	//should we limit m to 6? Can we assume the player 
	//actually knows how to make the move?
	///}
	int freecells = 0;
	for (int row = 0; row < 4; ++row) {
		if (!GetCard(row, 0)) ++freecells;
	}
	int freecols = 0;
	for (int col = 1; col < NCOLS; ++col) {
		if (!LastOfCol(col)) ++freecols;
	}
	
	if (to_empty_column) --freecols; //so we can use the 'non-empty' table
	if (freecols < 0) return 0; //can't move to an empty column if there is none!
	
	int moves = 0;
	if (freecols == 0) moves = freecells + 1;
	else if (freecols == 1) moves = 2 * (freecells + 1);
	else if (freecols == 2) moves = 4 * (freecells + 1);
	else if (freecols == 3) moves = 8 * (freecells + 1);
	else moves = 12;
	if (moves > 12) moves = 12;
	return moves;
}
queue<tuple<int,int,int,int,int,int>>& CDeck::GetLastMoves() {
	return lastMoves_;
}
//Movement. Only these change the board
void CDeck::move_card(int srow, int scol, int drow, int dcol, int nmove) {
	cards_[drow][dcol] = GetCard(srow, scol);
	int card = cards_[srow][scol];
	cards_[srow][scol] = 0; //empty cell of source
	while (undo_ < commands_.size()) commands_.pop_back(); //reset redo
	commands_.push_back(make_tuple(card, srow, scol, drow, dcol, nmove));
	++undo_; //so that undo = commands_.size()
}
bool CDeck::move_undo() {
	while (true) {
		if (!undo_) return false;
		--undo_;
		auto [scard, drow, dcol, srow, scol, nmove] = commands_[undo_]; //swap
		//int scard = cards_[srow][scol];
		cards_[drow][dcol] = scard;
		cards_[srow][scol] = (!scol && srow >= 4 && Rank(scard) > 1) ? scard - 4 : 0;
		if (!nmove) break;
	};
	return true;
}
bool CDeck::move_redo() {
	//restores the command pointed by undo_
	while (true) {
		if (undo_ == size(commands_)) return false;
		auto [card, srow, scol, drow, dcol, nmove] = commands_[undo_];
		//cards_[drow][dcol] = cards_[srow][scol];
		cards_[drow][dcol] = card;
		cards_[srow][scol] = 0;
		++undo_;
		if (undo_ == size(commands_)) break;
		if (get<5>(commands_[undo_]) == 0) break; //the nmove 'field'
	};
	return true;
}
bool CDeck::Move(string command) {
	bool ret;
	int prev_undo = undo_;
	lastMoves_ = {};
	if (command == "undo") ret = move_undo();
	else if (command == "redo") ret = move_redo();
	else {
		auto [srow, scol, scard] = CharToRowColCard(command[0]); //source
		if (scard <= 0) return false; //empty or invalid source cell!
		auto [drow, dcol, dcard] = CharToRowColCard(command[1]); //destination
		if (dcard < 0) return false; //invalid destination cell!

		if (dcol >= 1 &&  dcol <= 8) { //destination is a column
			if (dcard) { //destination column is not empty
				//how many cards are we asked to move?
				int n = Rank(dcard) - Rank(scard);
				if (n < 1) return false;
				if (n > max_moves(false)) return false;
				if (scol == 0) n = 1; //the source is a freecell
				
				//are the cards in the right sequence of rank and color?
				if (n == 1) {
					if (Color(scard) == Color(dcard)) return false;
					if (Rank(scard) + 1 != Rank(dcard)) return false;
				}
				else {
					int up = srow;
					int down = srow;
					for (int i = 1; i < n; ++i) {
						--up;
						if (up < 0) return false;
						if (Rank(GetCard(up, scol)) != Rank(GetCard(down, scol)) + 1) return false;
						if (Color(GetCard(up, scol)) == Color(GetCard(down, scol))) return false;
						down = up;
					}
					//color of 1st card of sequence ok?
					if (Color(GetCard(down, scol)) == Color(dcard)) return false;
				}

				//everything ok, make the move
				for (int i = 0; i < n; ++i) {
					move_card(srow - n + 1 + i, scol, drow + i + 1, dcol, i);
				}
			}
			else { //destination column is empty
				//find how many cards in the source column are in the right sequence
				int n = 1; //at least
				if (scol > 0) { //only if the source is another column
					int up = srow;
					int down = srow;
					while (srow >= 0) {
						--up;
						if (up < 0) break;
						if (Rank(GetCard(up, scol)) != Rank(GetCard(down, scol)) + 1) break;
						if (Color(GetCard(up, scol)) == Color(GetCard(down, scol))) break;
						++n;
						down = up;
					}
					if (n > max_moves(true)) n = max_moves(true); //limit the number
				}

				//everything ok, make the move
				for (int i = 0; i < n; ++i) {
					move_card(srow  - n + 1 + i, scol, i, dcol, i);
				}
			}
		}

		else if (drow >= 0 && drow <= 3) { //destination is a freecell
			if (dcard) return false; //freecell already taken
			move_card(srow, scol, drow, 0, 0);
		}

		else if (drow >= 4) { //destination is home
			int top = GetCard(4 + Suit(scard), 0);
			if (!top) {
				if (Rank(scard) != 1) return false; //not an ace
			}
			else {
				if (Rank(scard) != Rank(top) + 1) return false; //not next in rank
			}
			move_card(srow, scol, 4 + Suit(scard), 0, 0);
		}

		else return false;
		
		make_automatic_moves();
		ret = true;
	}

	//here is the place to generate the vector describing all moves made...
	int start = 0;
	int end = 0;
	int dif = undo_ - prev_undo;
	if (dif > 0) { start = prev_undo; end = undo_; }
	else if (dif < 0) { start = undo_; end = prev_undo;	}

	for (int i = start; i < end; ++i) {
		if (i < commands_.size()) {
			if (dif > 0)
				lastMoves_.push(commands_[i]);
			else if (dif < 0) {
				//we're undoing: swap source and destination
				auto [card, srow, scol, drow, dcol, nmove] = commands_[i];
				lastMoves_.push(make_tuple(card, drow, dcol, srow, scol, nmove));
			}
		}
		else {
			cout << "ERROR in CDeck::Move(): invalid index for commands_[]" << endl;
		}
	}
	
	return ret;
}
void CDeck::make_automatic_moves() {
	while (true) {
		//find minimum rank in home
		int min_home = -1;
		for (int row = 4; row < 8; ++row) {
			int card = GetCard(row, 0);
			if (card) {
				if (min_home < 0) min_home = Rank(card);
				if (Rank(card) < min_home) min_home = Rank(card);
			}
			else min_home = 0;
		}

		bool stop = true;

		//collect candidates
		vector<int> cards {};
		for (int row = 0; row < 4; ++row) {
			int card = GetCard(row, 0);
			if (card) cards.push_back(card);
		}
		for (int col = 1; col < NCOLS; ++col) {
			int card = LastOfCol(col);
			if (card) cards.push_back(card);
		}

		//select candidates
		vector<int> send_home {};
		for (int card : cards) {
			if (Rank(card) == 2) {
				//straigh home if ace of same suit present
				for (int row = 4; row < 8; ++row) {
					int ace = GetCard(row, 0);
					if (ace && Suit(ace) == Suit(card)) send_home.push_back(card);
				}
			}
			else if (Rank(card) == min_home + 1) {
				send_home.push_back(card);
			}
		}

		//send candidates home
		if (send_home.size() > 0) {
			stop = false;
			for (int card : send_home) {
				move_card(Row(card), Col(card), 4 + Suit(card), 0, 0);
			}
		}
		if (stop) break;
	}
}
///}

///{ CMyDrawingArea
CMyDrawingArea::CMyDrawingArea() {
	//fill array of card images 
	const string ranks = "A23456789TJQK";
	const string suits = "CDHS";
	int i = 1;
	for (int r = 0; r < ranks.size(); ++r) {
		for (int s = 0; s < suits.size(); ++s) {
			stringstream filename;
			filename << "images2/" << ranks[r] << suits[s] << ".png";
			imagesOriginalSize_[i] = Gdk::Pixbuf::create_from_file(filename.str());
			if (!imagesOriginalSize_[i]) 
				cout << filename.str() << ": No image found." << endl;
			++i;
		}
	}

	//these two enable the DrawingArea to receive key and mouse click events
	set_can_focus(true);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::KEY_PRESS_MASK);

	//context menu
	MenuItem *mi;
	mi = manage(new MenuItem("_Select Game", true));
	mi->signal_activate().connect(sigc::mem_fun(*this, &CMyDrawingArea::OnMenuSelect));
	menuPopup_.append(*mi);

	mi = manage(new MenuItem("_About", true));
	mi->signal_activate().connect(sigc::mem_fun(*this, &CMyDrawingArea::OnMenuAbout));
	menuPopup_.append(*mi);

	menuPopup_.show_all();

	//this is the context menu key event
	signal_popup_menu().connect(
		[this] () {
			this->menuPopup_.popup_at_widget(this, Gdk::Gravity::GRAVITY_CENTER, 
					Gdk::Gravity::GRAVITY_CENTER, nullptr);
			return true;
		}
	);
}
void CMyDrawingArea::Init() {
	ScaleBoard(0.16);

	//set up timer
	sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*this, &CMyDrawingArea::onTick), 0);
	sigc::connection conn = Glib::signal_timeout().connect(my_slot, 8); //milliseconds
	
	deck_ = nullptr;
	gameno_ = 1;

	//push a 'new game' command
	pendingMoves_.push(make_tuple(-4,0,0,0,0,0));
}
void CMyDrawingArea::ScaleBoard(double new_scale) {
	//For 0.16: 691,1056 -> 110,168
	scaleCard_ = new_scale;
	wCard_ = scaleCard_ * imagesOriginalSize_[1]->get_width(); 
	hCard_ = scaleCard_ * imagesOriginalSize_[1]->get_height();
	for (int i = 1; i <= NCARDS; ++i) {
		images_[i] = imagesOriginalSize_[i]->scale_simple(wCard_, hCard_, Gdk::INTERP_BILINEAR);
	}

	int xtop = 15; //top left corner of 'A' 
	int ytop = 10; //top left corner of 'A' 
	int gap = wCard_ / 10; //space between columns
	int ystep = 25 * hCard_ / 100; //visible part of card below

	//calculate x[][] and y[][]
	//column 0: freecells and home (top row)
	int x = xtop, y = ytop; 
	for (int row = 0; row < 8; ++row) {
		pos_[row][0] = make_tuple(x,y);
		x = x + wCard_ + gap;
	}
	//columns 1 to 8
	y = ytop + hCard_ + gap; x = xtop; //top left corner of first card in column '1' 
	for (int row = 0; row < NROWS; ++row) {
		for (int col = 1; col < NCOLS; ++col) {
			pos_[row][col] = make_tuple(x,y);
			x = x + wCard_ + gap;
		}
		x = xtop + 2*row; y += ystep;
	}
	
	int width = 2*xtop + 8*wCard_ + 7*gap;
	RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
	int height = screen->get_height();
	set_size_request(width, height);

	RefPtr<Gdk::Window> w = get_window();
	w->resize(width, height);
	frame_ = w->create_similar_surface(Cairo::CONTENT_COLOR_ALPHA, width, height);
	background_ = w->create_similar_surface(Cairo::CONTENT_COLOR_ALPHA, width, height);
	g_App->GetWindow().resize(width, height);
}
bool CMyDrawingArea::on_button_press_event(GdkEventButton* button_event) {
	//Call base class to allow normal handling. Needed here? 
	bool ret = DrawingArea::on_button_press_event(button_event);
	//Then do our custom stuff:
	if ((button_event->type == GDK_BUTTON_PRESS) && (button_event->button == 3)) {
		menuPopup_.popup_at_pointer((GdkEvent*)button_event);
	}
	return ret;
}
bool CMyDrawingArea::on_key_press_event(GdkEventKey* event) {
	guint k = event->keyval;

	string command = {};

	if (selectedCard_) command += deck_->CardToChar(selectedCard_);
	
	if (k >= GDK_KEY_1 && k <= GDK_KEY_8) {
		int col = k - GDK_KEY_0; //1..8
		if (selectedCard_) command += ('0' + col);
		else pendingMoves_.push(make_tuple(deck_->LastOfCol(col),0,0,0,0,0));
	}
	else if (k >= GDK_KEY_a && k <= GDK_KEY_d) {
		int row = k - GDK_KEY_a; //0..3
		if (selectedCard_) command += ('A' + row);
		else pendingMoves_.push(make_tuple(deck_->GetCard(row, 0),0,0,0,0,0));
	}
	else if (k == GDK_KEY_h) command += 'H';
	else if (k == GDK_KEY_BackSpace) { 
		if (!selectedCard_) command = "undo";
		pendingMoves_.push(make_tuple(0,0,0,0,0,0)); //de-select
	}
	else if (k == GDK_KEY_Tab) { 
		command = "redo"; //selectCard(0); 
	}
	else if (k == GDK_KEY_Escape) pendingMoves_.push(make_tuple(0,0,0,0,0,0));
	else if (k == GDK_KEY_plus)	pendingMoves_.push(make_tuple(-2,0,0,0,0,0));
	else if (k == GDK_KEY_minus) pendingMoves_.push(make_tuple(-3,0,0,0,0,0));
	else return DrawingArea::on_key_press_event(event);

	if (command.size() > 1) { //a command for CDeck
		if (deck_->Move(command)) pendingMoves_.push(make_tuple(0,0,0,0,0,0));
		//add the moves to our animation queue
		queue<tuple<int,int,int,int,int,int>> m = deck_->GetLastMoves();
		while (!m.empty()) {
			pendingMoves_.push(m.front());
			m.pop();
		}
	}
}
void CMyDrawingArea::OnMenuAbout() {
	Dialog* dlg = new Dialog("About me...", 
		(Gtk::Window&)(*this->get_toplevel()), 
		Gtk::DIALOG_MODAL
	);
	dlg->add_button("_OK", RESPONSE_OK);
	
	Label* lbl = manage(new Label());
	lbl->set_markup(
		"<big><b>Cartita</b></big>\n"
		"<i>"
		"Freecell Solitaire Game\n"
		"Following Microsoft's\n"
		"Game Numbering System\n"
		"</i>"
		"<small> Version: 1.0</small>"
	);
	
	Image* img = manage(new Image("card_ace.png"));
	Grid* g = manage(new Grid());
	g->set_border_width(20);
	g->set_row_spacing(20);
	g->set_column_spacing(20);
	g->attach(*img, 0, 0, 1, 1);
	g->attach(*lbl, 1, 0, 1, 1);
	
	Box* b = dlg->get_content_area();
	b->pack_start(*g, true, true, 25);

	dlg->show_all();
	dlg->run();
	delete dlg;
	return; 
}
void CMyDrawingArea::OnMenuSelect() {
	Dialog* dlg = new Dialog("Select Game", 
		(Gtk::Window&)(*this->get_toplevel()), 
		Gtk::DIALOG_MODAL
	);
	dlg->add_button("_OK", RESPONSE_OK);
	dlg->add_button("_Cancel", RESPONSE_CANCEL);
	dlg->set_default_response(RESPONSE_OK);
	
	Entry* e = manage(new Entry());
	long int gameno = 0;
	
	Grid* g = manage(new Grid());
	g->set_border_width(20);
	g->set_row_spacing(20);
	g->set_column_spacing(20);
	g->attach(*e, 0, 0, 1, 1);
	
	Box* b = dlg->get_content_area();
	b->pack_start(*g, true, true, 25);

	dlg->show_all();
	e->set_text(to_string(gameno_));
	e->grab_focus();
	while (dlg->run() == RESPONSE_OK) {
		string s = e->get_text();
		if (s.size() < 12) {
			string error;
			try {
				long int l = stol(s.c_str());
				gameno = l;
			}
			catch (std::invalid_argument const &e) {
				MessageDialog* dlg_warn = new MessageDialog(*dlg, 
					"Invalid game number",
					true, MESSAGE_ERROR, BUTTONS_OK, true
				);
				dlg_warn->run();
				delete dlg_warn; //all this screws amnt tab???!!!
			}
			if (gameno > 0) break;
		}
		e->grab_focus();
	}
	delete dlg;
	if (gameno > 0 && gameno != gameno_) {
		gameno_ = gameno;
		pendingMoves_.push(make_tuple(-4,0,0,0,0,0));
	}
}
bool CMyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	if (frame_) cr->set_source(frame_, 0, 0);
	cr->paint();
	return true;
}
void CMyDrawingArea::DrawBoard(int skip_card) {
	Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(frame_);
	cr->set_source_rgb(0.0, 0.2, 0.0); //background
	cr->paint(); 
	for (int col = 0; col < NCOLS; ++col) {
		int nrows = col ? NROWS : 8;
		for (int row = 0; row < nrows; ++row) {
			int card = cards_[row][col];
			if (card && card == skip_card) {
				card = (!col && row >= 4 && deck_->Rank(card) > 1) ? card - 4 : 0;
			}
			if (card) {
				auto [x,y] = pos_[row][col];
				Gdk::Cairo::set_source_pixbuf(cr, images_[card], x, y);
				cr->paint();
				if (card == selectedCard_) { //highlight card
					cr->set_source_rgba(0.5, 0.1, 0.1, 0.4);
					cr->rectangle(x, y, wCard_, hCard_);
					cr->fill();
				}
			}
		}
	}
}
bool CMyDrawingArea::onTick(int param) {
	static bool moving = false;
	static queue<tuple<int,int>> path = {};
	static int moving_card;

	if (!moving) {
		if (pendingMoves_.empty()) return true;
		auto [card, srow, scol, drow, dcol, nmove] = pendingMoves_.front();
		
		if (!srow && !scol && !drow && !dcol && !nmove) { //special commands
			if (card >= 0) selectedCard_ = card;
			else if (card == -2) ScaleBoard(GetScale() + 0.01);
			else if (card == -3) ScaleBoard(GetScale() - 0.01);
			else if (card == -4) { //new game
				if (deck_) delete deck_;
				deck_ = new CDeck(gameno_);
				//load our local copy of CDeck's cards_[][]
				for (int col = 0; col < NCOLS; ++col) {
					int nrows = col ? NROWS : 8;
					for (int row = 0; row < nrows; ++row) 
						cards_[row][col] = deck_->GetCard(row, col);
				}
				//push a 'select card' command, which in turn will draw the board
				stringstream title;
				title << "Game #" << gameno_; 
				g_App->GetWindow().set_title(title.str());
				pendingMoves_.push(make_tuple(0,0,0,0,0,0));				
			}
			pendingMoves_.pop();
			DrawBoard();
			queue_draw(); //invalidate entire drawing area
		}
		else { //move card: set up the move
			moving_card = card;
			DrawBoard(moving_card); //skip 'card'
			Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(background_);
			if (frame_) cr->set_source(frame_, 0, 0);
			cr->paint(); //this created a copy of the board to be used as background during the move
			auto [x1,y1] = pos_[srow][scol];
			auto [x2,y2] = pos_[drow][dcol];
			int dx = x2 - x1; if (dx < 0) dx *= -1;
			int dy = y2 - y1; if (dy < 0) dy *= -1;
			int xstep = dx / NSTEPS; if (x1 > x2) xstep *= -1;
			int ystep = dy / NSTEPS; if (y1 > y2) ystep *= -1;
			for (int i = 1; i < NSTEPS; ++i) path.push(make_tuple(x1 + i*xstep, y1 + i*ystep));
			path.push(make_tuple(x2, y2));
			moving = true;
		}
	}
	else {
		if (!path.empty()) { //draw the moving card according to path
			Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(frame_);
			if (background_) cr->set_source(background_, 0, 0);
			cr->paint(); //use the snapshot of the board created above as background
			auto [x,y] = path.front();
			Gdk::Cairo::set_source_pixbuf(cr, images_[moving_card], x, y);
			cr->paint();
			path.pop();
			queue_draw(); //invalidate entire drawing area
		}
		else { //end of path: update cards_[][]
			auto [card, srow, scol, drow, dcol, nmove] = pendingMoves_.front();
			cards_[drow][dcol] = cards_[srow][scol];
			int scard = cards_[srow][scol];
			cards_[srow][scol] = (!scol && srow >= 4 && deck_->Rank(scard) > 1) ? 
				scard - 4 : 0;
			pendingMoves_.pop();
			moving = false;
		}
	}
	return true; //keep ticking
}
CMyDrawingArea::~CMyDrawingArea() {
}
///}

///{ CMyWindow
CMyWindow::CMyWindow() {
	set_title("Cartita");
	set_icon_from_file("card_ace.png");
	add(area_);
	show_all_children();
}
CMyWindow::~CMyWindow() {
}
///} MyWindow

///{ CMyApp
CMyApp::CMyApp() : Application("org.gtkmm.cards4.application") {
	Glib::set_application_name("Cartita");

	//Change to exe dir
	char *buff = new char[1024];
	int ret;
	
	#ifdef __linux__
		cout << "Running on Linux..." << endl;
		ret = readlink("/proc/self/exe", buff, 1023);
		if (ret == -1)
			cout << "readlink failed!" << endl;
		else 
			buff[ret] = '\0';
	#endif
	#ifdef __WIN32
		cout << "Running on Windows..." << endl;
		ret = GetModuleFileName(0, buff, 1023);
		if (!ret) cout << "GetModuleFilename failed!" << endl;
	#endif

	if (ret != -1 && ret != 0) {
		int pos = string(buff).find_last_of("/\\");
		chdir(string(buff).substr(0, pos).c_str());
	}
	
	getcwd(buff, 1023);
	cout << "Current directory: " << buff << endl;
	delete[] buff;
}
void CMyApp::ApplyCss(Widget& w, const string& state, const string& css_string) {
	// examples:
	//color:#ff00ea; font:12px "Comic Sans";
	//background:white; caret-color:black;
	string css = "#" + w.get_name() + (state.empty() ? "" : ":" + state) +
		" {" + css_string + "}";
	RefPtr<CssProvider> refCssProvider = CssProvider::create();
	refCssProvider->load_from_data(css);
	RefPtr<StyleContext> refStyleContext = w.get_style_context();
	refStyleContext->add_provider(refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
CMyWindow& CMyApp::GetWindow() {
	return *window_;
}
void CMyApp::on_activate() {
	window_ = new CMyWindow();
	add_window(*window_);
	window_->signal_hide().connect(
		[this] () {
			delete this->window_; //delete when hidden
		}
	);
	window_->present();
	window_->area_.Init();
}
///} CMyApp

int main(int argc, char *argv[]) {
	cout << termcolor::red << "♥";
	cout << termcolor::reset << "♣";
	cout << termcolor::red << "♦";
	cout << termcolor::reset << "♠" << endl;
	g_App = RefPtr<CMyApp>(new CMyApp());
	return g_App->run(argc, argv);
}