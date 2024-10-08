#ifndef LTJS_DINPUT_INCLUDED
#define LTJS_DINPUT_INCLUDED


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

constexpr auto DIK_ESCAPE = 0x01;
constexpr auto DIK_1 = 0x02; // On main keyboard
constexpr auto DIK_2 = 0x03; // On main keyboard
constexpr auto DIK_3 = 0x04; // On main keyboard
constexpr auto DIK_4 = 0x05; // On main keyboard
constexpr auto DIK_5 = 0x06; // On main keyboard
constexpr auto DIK_6 = 0x07; // On main keyboard
constexpr auto DIK_7 = 0x08; // On main keyboard
constexpr auto DIK_8 = 0x09; // On main keyboard
constexpr auto DIK_9 = 0x0A; // On main keyboard
constexpr auto DIK_0 = 0x0B; // On main keyboard
constexpr auto DIK_MINUS = 0x0C; // On main keyboard
constexpr auto DIK_EQUALS = 0x0D; // On main keyboard
constexpr auto DIK_BACK = 0x0E; // BACKSPACE
constexpr auto DIK_TAB = 0x0F;
constexpr auto DIK_Q = 0x10;
constexpr auto DIK_W = 0x11;
constexpr auto DIK_E = 0x12;
constexpr auto DIK_R = 0x13;
constexpr auto DIK_T = 0x14;
constexpr auto DIK_Y = 0x15;
constexpr auto DIK_U = 0x16;
constexpr auto DIK_I = 0x17;
constexpr auto DIK_O = 0x18;
constexpr auto DIK_P = 0x19;
constexpr auto DIK_LBRACKET = 0x1A; // Left square bracket [
constexpr auto DIK_RBRACKET = 0x1B; // Right square bracket ]
constexpr auto DIK_RETURN = 0x1C; // ENTER on main keyboard
constexpr auto DIK_LCONTROL = 0x1D; // Left CTRL
constexpr auto DIK_A = 0x1E;
constexpr auto DIK_S = 0x1F;
constexpr auto DIK_D = 0x20;
constexpr auto DIK_F = 0x21;
constexpr auto DIK_G = 0x22;
constexpr auto DIK_H = 0x23;
constexpr auto DIK_J = 0x24;
constexpr auto DIK_K = 0x25;
constexpr auto DIK_L = 0x26;
constexpr auto DIK_SEMICOLON = 0x27;
constexpr auto DIK_APOSTROPHE = 0x28;
constexpr auto DIK_GRAVE = 0x29; // Grave accent (`)
constexpr auto DIK_LSHIFT = 0x2A; // Left SHIFT
constexpr auto DIK_BACKSLASH = 0x2B;
constexpr auto DIK_Z = 0x2C;
constexpr auto DIK_X = 0x2D;
constexpr auto DIK_C = 0x2E;
constexpr auto DIK_V = 0x2F;
constexpr auto DIK_B = 0x30;
constexpr auto DIK_N = 0x31;
constexpr auto DIK_M = 0x32;
constexpr auto DIK_COMMA = 0x33;
constexpr auto DIK_PERIOD = 0x34; // On main keyboard
constexpr auto DIK_SLASH = 0x35; // Forward slash (/) on main keyboard
constexpr auto DIK_RSHIFT = 0x36; // Right SHIFT
constexpr auto DIK_MULTIPLY = 0x37; // Asterisk (*) on numeric keypad
constexpr auto DIK_LMENU = 0x38; // Left ALT
constexpr auto DIK_SPACE = 0x39; // SPACEBAR
constexpr auto DIK_CAPITAL = 0x3A; // CAPS LOCK
constexpr auto DIK_F1 = 0x3B;
constexpr auto DIK_F2 = 0x3C;
constexpr auto DIK_F3 = 0x3D;
constexpr auto DIK_F4 = 0x3E;
constexpr auto DIK_F5 = 0x3F;
constexpr auto DIK_F6 = 0x40;
constexpr auto DIK_F7 = 0x41;
constexpr auto DIK_F8 = 0x42;
constexpr auto DIK_F9 = 0x43;
constexpr auto DIK_F10 = 0x44;
constexpr auto DIK_NUMLOCK = 0x45;
constexpr auto DIK_SCROLL = 0x46; // SCROLL LOCK
constexpr auto DIK_NUMPAD7 = 0x47;
constexpr auto DIK_NUMPAD8 = 0x48;
constexpr auto DIK_NUMPAD9 = 0x49;
constexpr auto DIK_SUBTRACT = 0x4A; // MINUS SIGN (-) on numeric keypad
constexpr auto DIK_NUMPAD4 = 0x4B;
constexpr auto DIK_NUMPAD5 = 0x4C;
constexpr auto DIK_NUMPAD6 = 0x4D;
constexpr auto DIK_ADD = 0x4E; // PLUS SIGN (+) on numeric keypad
constexpr auto DIK_NUMPAD1 = 0x4F;
constexpr auto DIK_NUMPAD2 = 0x50;
constexpr auto DIK_NUMPAD3 = 0x51;
constexpr auto DIK_NUMPAD0 = 0x52;
constexpr auto DIK_DECIMAL = 0x53; // PERIOD (decimal point) on numeric keypad
//constexpr auto DIK_OEM_102 = 0x56; // On British and German keyboards
constexpr auto DIK_F11 = 0x57;
constexpr auto DIK_F12 = 0x58;
constexpr auto DIK_F13 = 0x64; // On NEC PC-98 Japanese keyboard
constexpr auto DIK_F14 = 0x65; // On NEC PC-98 Japanese keyboard
constexpr auto DIK_F15 = 0x66; // On NEC PC-98 Japanese keyboard
//constexpr auto DIK_KANA = 0x70; // On Japanese keyboard
//constexpr auto DIK_ABNT_C1 = 0x73; // On numeric pad of Brazilian keyboards
//constexpr auto DIK_CONVERT = 0x79; // On Japanese keyboard
//constexpr auto DIK_NOCONVERT = 0x7B; // On Japanese keyboard
//constexpr auto DIK_YEN = 0x7D; // On Japanese keyboard
//constexpr auto DIK_ABNT_C2 = 0x7E; // On numeric pad of Brazilian keyboards
//constexpr auto DIK_NUMPADEQUALS = 0x8D; // On numeric keypad of NEC PC-98 Japanese keyboard
//constexpr auto DIK_PREVTRACK = 0x90; // Previous track; circumflex on Japanese keyboard
//constexpr auto DIK_AT = 0x91; // On Japanese keyboard
//constexpr auto DIK_COLON = 0x92; // On Japanese keyboard
//constexpr auto DIK_UNDERLINE = 0x93; // On NEC PC-98 Japanese keyboard
//constexpr auto DIK_KANJI = 0x94; // On Japanese keyboard
//constexpr auto DIK_STOP = 0x95; // On NEC PC-98 Japanese keyboard
//constexpr auto DIK_AX = 0x96; // On Japanese keyboard
//constexpr auto DIK_UNLABELED = 0x97; // On Japanese keyboard
//constexpr auto DIK_NEXTTRACK = 0x99; // Next track
constexpr auto DIK_NUMPADENTER = 0x9C;
constexpr auto DIK_RCONTROL = 0x9D; // Right CTRL
//constexpr auto DIK_MUTE = 0xA0;
//constexpr auto DIK_CALCULATOR = 0xA1;
//constexpr auto DIK_PLAYPAUSE = 0xA2;
//constexpr auto DIK_MEDIASTOP = 0xA4;
//constexpr auto DIK_VOLUMEDOWN = 0xAE;
//constexpr auto DIK_VOLUMEUP = 0xB0;
//constexpr auto DIK_WEBHOME = 0xB2;
//constexpr auto DIK_NUMPADCOMMA = 0xB3; // On numeric keypad of NEC PC-98 Japanese keyboard
constexpr auto DIK_DIVIDE = 0xB5; // Forward slash (/) on numeric keypad
constexpr auto DIK_SYSRQ = 0xB7;
constexpr auto DIK_RMENU = 0xB8; // Right ALT
constexpr auto DIK_PAUSE = 0xC5;
constexpr auto DIK_HOME = 0xC7;
constexpr auto DIK_UP = 0xC8; // UP ARROW
constexpr auto DIK_PRIOR = 0xC9; // PAGE UP
constexpr auto DIK_LEFT = 0xCB; // LEFT ARROW
constexpr auto DIK_RIGHT = 0xCD; // RIGHT ARROW
constexpr auto DIK_END = 0xCF;
constexpr auto DIK_DOWN = 0xD0; // DOWN ARROW
constexpr auto DIK_NEXT = 0xD1; // PAGE DOWN
constexpr auto DIK_INSERT = 0xD2;
constexpr auto DIK_DELETE = 0xD3;
//constexpr auto DIK_LWIN = 0xDB; // Left Windows logo key
//constexpr auto DIK_RWIN = 0xDC; // Right Windows logo key
//constexpr auto DIK_APPS = 0xDD; // Application key
//constexpr auto DIK_POWER = 0xDE;
//constexpr auto DIK_SLEEP = 0xDF;
//constexpr auto DIK_WAKE = 0xE3;
//constexpr auto DIK_WEBSEARCH = 0xE5;
//constexpr auto DIK_WEBFAVORITES = 0xE6; // Displays the Microsoft Internet Explorer Favorites list, the Windows Favorites folder, or the Netscape Bookmarks list.
//constexpr auto DIK_WEBREFRESH = 0xE7;
//constexpr auto DIK_WEBSTOP = 0xE8;
//constexpr auto DIK_WEBFORWARD = 0xE9;
//constexpr auto DIK_WEBBACK = 0xEA;
//constexpr auto DIK_MYCOMPUTER = 0xEB;
//constexpr auto DIK_MAIL = 0xEC;
//constexpr auto DIK_MEDIASELECT = 0xED; // Media Select key, which displays a selection of supported media players on the system

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#endif // !LTJS_DINPUT_INCLUDED
