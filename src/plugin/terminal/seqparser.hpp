/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 *
 * SDLTerminal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SDLTerminal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with SDLTerminal.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SEQPARSER_HPP__
#define SEQPARSER_HPP__

#include <string.h>

#include <list>
#include <map>

#include "util/strcmp.hpp"

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
} CSDeviceAttr_t;

typedef enum
{
	CS_DS_READY = 0,
	CS_DS_BAD = 3,
	CS_DS_STATUS_REPORT = 5,
	CS_DS_POS_REPORT = 6
} CSDeviceStatus_t;

typedef enum
{
	CS_ED_CUR_TO_END = 0,
	CS_ED_START_TO_CUR,
	CS_ED_ALL
} CSEraseDisplay_t;

typedef enum
{
	CS_EL_CUR_TO_END = 0,
	CS_EL_START_TO_CUR,
	CS_EL_ALL
} CSEraseLine_t;

typedef enum
{
	CS_GM_NONE = 0,
	CS_GM_BOLD = 1,
	CS_GM_UNDERSCORE = 4,
	CS_GM_BLINK = 5,
	CS_GM_NEGATIVE = 7
} CSGraphicsMode_t;

typedef enum
{
	CS_TC_CUR = 0,
	CS_TC_ALL = 3
} CSTABClear_t;

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
	CS_TM_LINE_FEED_NEW_LINE = 20
} CSTermMode_t;

typedef enum
{
	CS_UNKNOWN,

	//ANSI
	CS_CURSOR_POSITION_REPORT, //ESC[<Line>;<Column>R
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
	CS_MODE_SET, //ESC[<?><Value>;...;<Value>h
	CS_MODE_RESET, //ESC[<?><Value>;...;<Value>l

	CS_KEYPAD_APP_MODE, //ESC=
	CS_KEYPAD_NUM_MODE, //ESC>

	CS_USER_MAPPING, // USER MAPPING ??

	CS_CHARSET_UK_G0_SET, //ESC(A
	CS_CHARSET_UK_G1_SET, //ESC)A
	CS_CHARSET_US_G0_SET, //ESC(B
	CS_CHARSET_US_G1_SET, //ESC)B
	CS_CHARSET_SPEC_G0_SET, //ESC(0
	CS_CHARSET_SPEC_G1_SET, //ESC)0
	CS_CHARSET_ALT_G0_SET, //ESC(1
	CS_CHARSET_ALT_G1_SET, //ESC)1
	CS_CHARSET_ALT_SPEC_G0_SET, //ESC(2
	CS_CHARSET_ALT_SPEC_G1_SET, //ESC)2

	CS_MARGIN_SET, //ESC[<Top>;<Bottom>r
	CS_MOVE_UP, //ESCD
	CS_MOVE_DOWN, //ESCM
	CS_MOVE_NEXT_LINE, //ESCE

	CS_TAB, //ESCH
	CS_TAB_CLEAR, //ESC[<Value>;...;<Value>g

	CS_DOUBLE_HEIGHT_LINE_TOP, //ESC#3
	CS_DOUBLE_HEIGHT_LINE_BOTTOM, //ESC#4
	CS_SINGLE_WIDTH_LINE, //ESC#5
	CS_DOUBLE_WIDTH_LINE, //ESC#6

	CS_DEVICE_STATUS_REPORT, //ESC[<Value>;...;<Value>n
	CS_DEVICE_ATTR_PRIMARY_REQUEST, //ESC[<Value>c
	CS_DEVICE_ATTR_PRIMARY_RESPONSE, //ESC[?1;<Value>c

	CS_DEVICE_ATTR_SECONDARY_REQUEST, //ESC[><Value>c
	CS_DEVICE_ATTR_SECONDARY_RESPONSE, //ESC[>1;<Value>c

	CS_TERM_IDENTIFY, //ESCZ
	CS_TERM_PARAM, //ESC[<Value>;...;<Value>x
	CS_TERM_RESET, //ESCc

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

	CS_MAX
} CSToken_t;

typedef struct
{
	const char *m_sCSI;
	CSToken_t m_token;
	int m_nMinParams; //Minimum number of parameters should be 0.
	int m_nMaxParams; //-1 for variable parameters.
	int m_nDefaultVal; //-1 for not applicable.
	char m_cFinal;
} CSEntry_t;

class ControlSeqParser
{
private:
	static const char ESC_CHAR;
	static const char DELIMITER_CHAR;

	std::map<const char *, std::list<CSEntry_t *> *, cmp_str> m_csLookup;
	int *m_values;
	int m_numValues;
	int m_savedPos;
	int m_currentPos;
	char m_currentChar;
	const char *m_seq;

	int parsePositiveInt(int *values, int numMaxValues);
	int parseSeq();

	int match(char *prefix, char suffix, int *values, int numValues);
	bool isPartialMatch(char *prefix);
	bool isPartialMatch(char *prefix, int *values, int numValues);

	void freeLookup();
	void buildLookup();
	void addLookupEntry(const char *sCSI, CSToken_t token, int nMinParam, int nMaxParam, int nDefaultVal, char cFinal);

	void nextChar();
	void reset();
public:
	static const int MAX_NUM_VALUES;

	ControlSeqParser();
	~ControlSeqParser();
	int parse(const char *seq, int *values, int *numValues, int *seqLength);
};

#endif
