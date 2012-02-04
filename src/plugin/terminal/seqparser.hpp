/**
 * This file is part of wTerm.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 * Copyright (C) 2011-2012 Ryan Hope <rmh3093@gmail.com>
 *
 * wTerm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wTerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with wTerm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SEQPARSER_HPP__
#define SEQPARSER_HPP__

#include <string.h>

#include <list>
#include <map>
#include <string>

#include "terminalstate.hpp"

/* unused:
typedef enum
{
	CS_DA_NONE = 0,
	CS_DA_STP,
	CS_DA_AVO,
	CS_DA_AVO_STP,
	CS_DA_GPO,
	CS_DA_GPO_STP,
	CS_DA_GPO_AVO,
	CS_DA_GPO_STP_AVO
} CSDeviceAttr;

typedef enum
{
	CS_DS_READY = 0,
	CS_DS_BAD = 3,
	CS_DS_STATUS_REPORT = 5,
	CS_DS_POS_REPORT = 6
} CSDeviceStatus;

typedef enum
{
	CS_ED_CUR_TO_END = 0,
	CS_ED_START_TO_CUR,
	CS_ED_ALL
} CSEraseDisplay;

typedef enum
{
	CS_EL_CUR_TO_END = 0,
	CS_EL_START_TO_CUR,
	CS_EL_ALL
} CSEraseLine;

typedef enum
{
	CS_GM_NONE = 0,
	CS_GM_BOLD = 1,
	CS_GM_UNDERSCORE = 4,
	CS_GM_BLINK = 5,
	CS_GM_NEGATIVE = 7
} CSGraphicsMode;

typedef enum
{
	CS_TC_CUR = 0,
	CS_TC_ALL = 3
} CSTABClear;

typedef enum
{
	CS_TM_ERROR = 0,
	CS_TM_CURSOR_KEY,
	CS_TM_ANSI_VT52,
	CS_TM_COLUMN,
	CS_TM_SCROLLING,
	CS_TM_SCREEN,
	CS_TM_ORIGIN,
	CS_TM_AUTO_WRAP,
	CS_TM_AUTO_REPEAT,
	CS_TM_INTERLACE,
	CS_TM_CURSOR,
	CS_TM_LINE_FEED_NEW_LINE = 20
} CSTermMode;
*/

typedef enum
{
	CS_UNKNOWN, // normal character

	//ASCII
	CS_ASCII_BEL, //Bell
	CS_ASCII_BS, //Backspace
	CS_ASCII_TAB, //Tab
	CS_ASCII_LF, //Linefeed
	CS_ASCII_VT, //Vertical tab
	CS_ASCII_FF, //Form feed
	CS_ASCII_CR, //Carriage return

	//ANSI
	CS_CURSOR_POSITION, //ESC[<Line>;<Column>H or ESC[<Line>;<Column>f
	CS_CURSOR_UP, //ESC[<Value>A
	CS_CURSOR_DOWN, //ESC[<Value>B
	CS_CURSOR_FORWARD, //ESC[<Value>C
	CS_CURSOR_BACKWARD, //ESC[<Value>D
	CS_CURSOR_POSITION_SAVE, //ESC[s or ESC7
	CS_CURSOR_POSITION_RESTORE, //ESC[u or ESC8
	CS_ERASE_DISPLAY, //ESC[<Value>;...;<Value>J
	CS_ERASE_LINE, //ESC[<Value>;...;<Value>K
	CS_GRAPHICS_MODE_SET, //ESC[<Value>;...;<Value>m

	CS_MODE_SET, //ESC[<Value>;...;<Value>h
	CS_MODE_RESET, //ESC[<Value>;...;<Value>l

	CS_MODE_SET_PRIV, //ESC[<?><Value>;...;<Value>h
	CS_MODE_RESET_PRIV, //ESC[<?><Value>;...;<Value>l

	CS_INDEX, //ESCD INDEX
	CS_REVERSE_INDEX, //ESCM REVERSE INDEX
	CS_TAB_SET, //ESCH TAB SET
	CS_TAB_CLEAR, //ESC[<Value>g

	CS_VPA, //ESC[<Value>d Line Position Absolute
	CS_CHA, //ESC[<Value>G Cursor Character Absolute
	CS_ECH, //ESC[<Value>X Erase Characters
	CS_IL, //ESC[<Value>L Insert Lines
	CS_DL, //ESC[<Value>M Delete Lines
	CS_DCH, //ESC[<Value>P Delete Characters
	CS_ICH, //ESC[<Value>@ Insert Blank Characters
	CS_HPA, //ESC[<Value>` Character Position Absolute
	CS_CBT, //ESC[<Value>Z Cursor Backward Tabulation
	CS_CNL, //ESC[<Value>E Cursor Next Line
	CS_CPL, //ESC[<Value>F Cursor Preceding Line
	CS_SU, //ESC[<Value>S Scroll Up Lines
	CS_SD, //ESC[<Value>T Scroll Down Lines

	CS_KEYPAD_APP_MODE, //ESC=
	CS_KEYPAD_NUM_MODE, //ESC>

	CS_USER_MAPPING, // USER MAPPING ??

	CS_CHARSET_UK_G0_SET, //ESC(A
	CS_CHARSET_ASCII_G0_SET, //ESC(B
	CS_CHARSET_SPEC_G0_SET, //ESC(0
	CS_CHARSET_ALT_G0_SET, //ESC(1
	CS_CHARSET_ALT_SPEC_G0_SET, //ESC(2
	CS_CHARSET_USE_G0, //ESCO

	CS_CHARSET_UK_G1_SET, //ESC)A
	CS_CHARSET_ASCII_G1_SET, //ESC)B
	CS_CHARSET_SPEC_G1_SET, //ESC)0
	CS_CHARSET_ALT_G1_SET, //ESC)1
	CS_CHARSET_ALT_SPEC_G1_SET, //ESC)2
	CS_CHARSET_USE_G1, //ESCN

	CS_CHARSET_UK_G2_SET, //ESC*A
	CS_CHARSET_ASCII_G2_SET, //ESC*B
	CS_CHARSET_SPEC_G2_SET, //ESC*0
	CS_CHARSET_ALT_G2_SET, //ESC*1
	CS_CHARSET_ALT_SPEC_G2_SET, //ESC*2
	CS_CHARSET_USE_G2, //ESCn

	CS_CHARSET_UK_G3_SET, //ESC+A
	CS_CHARSET_ASCII_G3_SET, //ESC+B
	CS_CHARSET_SPEC_G3_SET, //ESC+0
	CS_CHARSET_ALT_G3_SET, //ESC+1
	CS_CHARSET_ALT_SPEC_G3_SET, //ESC+2
	CS_CHARSET_USE_G3, //ESCo

	CS_MARGIN_SET, //ESC[<Top>;<Bottom>r
	CS_MOVE_UP, //ESCD
	CS_MOVE_DOWN, //ESCM
	CS_MOVE_NEXT_LINE, //ESCE
	CS_TAB_FORWARD, //ESC[<Value>I

	CS_DOUBLE_HEIGHT_LINE_TOP, //ESC#3
	CS_DOUBLE_HEIGHT_LINE_BOTTOM, //ESC#4
	CS_SINGLE_WIDTH_LINE, //ESC#5
	CS_DOUBLE_WIDTH_LINE, //ESC#6
	CS_SCREEN_ALIGNMENT_DISPLAY, //ESC#8

	CS_DEVICE_STATUS_REPORT, //ESC[<Value>;...;<Value>n
	CS_DEVICE_ATTR_PRIMARY_REQUEST, //ESC[<Value>c

	CS_DEVICE_ATTR_SECONDARY_REQUEST, //ESC[><Value>c

	CS_TERM_IDENTIFY, //ESCZ
	CS_TERM_PARAM, //ESC[<Value>;...;<Value>x
	CS_TERM_RESET, //ESCc

	CS_OSC, // Operating System Controls: ESC]<Number>;<String>BEL

	//Hardware debug sequences.
	CS_SCREEN_ALIGN_DISPLAY, //ESC#8
	CS_LED_LOAD, //ESC[<Value>;...;<Value>q
	CS_INVOKE_CONF_TEST, //ESC[2;<Value>;...;<Value>y

	//VT52
	CS_VT52_CURSOR_UP, //ESCA
	CS_VT52_CURSOR_DOWN, //ESCB
	CS_VT52_CURSOR_RIGHT, //ESCC
	CS_VT52_CURSOR_LEFT, //ESCD
	CS_VT52_SPEC_CHARSET, //ESCF
	CS_VT52_ASCII_CHARSET, //ESCG
	CS_VT52_CURSOR_HOME, //ESCH
	CS_VT52_REVERSE_LINE_FEED, //ESCI
	CS_VT52_ERASE_SCREEN, //ESCJ
	CS_VT52_ERASE_LINE, //ESCK
	CS_VT52_CURSOR_POSITION, //ESCY <line><Column>
	CS_VT52_IDENTIFY, //ESCZ
	CS_VT52_KEYPAD_ALT_MODE, //ESC=
	CS_VT52_KEYPAD_NORMAL_MODE, //ESC>
	CS_VT52_ANSI_MODE, //ESC<

	CS_BACK_INDEX, // ESC6
	CS_FORWARD_INDEX, // ESC9

	CS_INSERT_COLUMN, // ESC[<Value>'}
	CS_DELETE_COLUMN, // ESC[<Value>'~

	CS_SCROLL_RIGHT, // ESC[<Value> @
	CS_SCROLL_LEFT, // ESC[<Value> A

	CS_CURSOR_STYLE, // ESC[<Value> SPq

	CS_MAX
} CSToken;

class ControlSeqParser
{
public:
	typedef enum { MODE_7BIT, MODE_8BIT, MODE_UTF8 } Mode;

private:
	static const unsigned char ESC_CHAR;
	static const unsigned char DELIMITER_CHAR;
	static const int MAX_NUM_VALUES = 20;

	struct CSI_Entry {
		CSToken token;

		unsigned char parameter;
		unsigned char function;
		unsigned char suffix;
		int minParams; //Minimum number of parameters should be 0.
		int maxParams; //-1 for variable parameters.
		int defaultVal; //-1 for not applicable.
	};

	typedef std::map<unsigned char, std::list<CSI_Entry> > CSI_Lookup;
	CSI_Lookup m_csiLookup; /* map by final function */

	struct CS_Fixed_Entry {
		CSToken token;
		unsigned char fixed[4];
	};

	typedef std::map<unsigned char, std::list<CS_Fixed_Entry> > CS_Fixed_Lookup;
	CS_Fixed_Lookup m_csFixedLookup; /* map by first char */
	CS_Fixed_Lookup m_csVT52Lookup; /* map by first char */

	int m_values[MAX_NUM_VALUES];
	int m_numValues;

	std::string m_oscString;

	CellCharacter m_currentChar;
	CSToken m_token;
	unsigned char m_suffix;
	unsigned char m_prefix[4];
	int m_prefixlen;
	enum { ST_START, ST_ESCAPE, ST_ESCAPE_TRIE, ST_CSI, ST_CSI_VALUES, ST_CSI_SUFFIX, ST_CSI_INVALID, ST_OSC, ST_OSC_ESC, ST_VT52_ESCY } m_state;

	const unsigned char *m_seq;
	int m_len;

	Mode m_mode;
	unsigned int m_utf8_seqlen, m_utf8_remlen;

	bool m_vt52;

	void buildLookup();

	void addFixedLookup(const char *str, CSToken token);
	void addVT52FixedLookup(const char *str, CSToken token);
	void addCSILookup(const char parameter, CSToken token, int nMinParam, int nMaxParam, int nDefaultVal, char cFinal);
	void addCSI2Lookup(const char parameter, CSToken token, int nMinParam, int nMaxParam, int nDefaultVal, char cSuffix, char cFinal);

	bool tryFixedEscape();

	void parseCSIValue();
	bool matchCSI();

	bool parseChar(); /* returns true if token complete */

	unsigned char nextByte();
	bool nextChar(); /* tries to build next "char" - may need to decode multiple bytes */
public:
	ControlSeqParser();
	~ControlSeqParser();

	/* don't free seq or call again until next() returned false */
	void addInput(const char *seq, int len);

	bool next(); /* returns false if more input is needed */

	CSToken token() const { return m_token; }
	CellCharacter character() const { return m_currentChar; }
	unsigned int numValues() const { return m_numValues; }
	int value(unsigned int idx) const { return m_values[idx]; }
	int* values() { return m_values; }

	std::string getOSCParameter() { return m_oscString; }

	void setMode(Mode mode);
	Mode getMode() const { return m_mode; }

	void enableVT52();
	void disableVT52();

	void reset();
};

#endif
