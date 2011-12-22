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

#include "terminalconfigmanager.hpp"

#include <stdlib.h>
#include <string.h>

TerminalConfigManager::TerminalConfigManager()
{
	memset(&m_keyBindings, 0, sizeof(m_keyBindings));
}

TerminalConfigManager::~TerminalConfigManager()
{
	freeKeyBindings();
}

void TerminalConfigManager::freeKeyBindings()
{
	std::map<int, int> *map;
	std::map<int, int>::iterator itr;

	for (int i = 0; i < TERM_KEYMOD_MAX; i++)
	{
		map = m_keyBindings[i];

		if (map != NULL)
		{
			delete m_keyBindings[i];
			m_keyBindings[i] = NULL;
		}
	}
}

int TerminalConfigManager::getKeyBinding(Term_KeyMod_t keyMod, int nInput)
{
	if (keyMod != TERM_KEYMOD_MAX)
	{
		std::map<int, int> *map = m_keyBindings[keyMod];
		std::map<int, int>::iterator locator;

		if (map == NULL)
		{
			generateDefaultKeyBindings();
			map = m_keyBindings[keyMod];
		}

		locator = map->find(nInput);

		if (locator != map->end())
		{
			return locator->second;
		}
	}

	return 0;
}

int TerminalConfigManager::parse(const char *sFileName)
{
	int nResult = ConfigManager::parse(sFileName);

	if (nResult == 0)
	{
		nResult = generateKeyBindings();
	}

	return nResult;
}

/**
 * The length of the string that was copied.
 */
int TerminalConfigManager::parseString(const char *src, int *values, int nMaxValues)
{
	int nIndex = 0;
	char c = 0;
	int nEscaped = 0; //State for parsing escaped character.
	int nOldEscaped = 0;
	int nValue = 0;
	int nLength = strlen(src) + 1;

	for (int i = 0; i < nLength; i++)
	{
		c = src[i];
		nOldEscaped = nEscaped;

		switch (c)
		{
		case '\\':
			if (nEscaped == 0)
			{
				nEscaped++;
			}
			break;
		case 'x':
			if (nEscaped == 1)
			{
				nEscaped++;
			}
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (nEscaped >= 2 && nEscaped <= 5)
			{
				nValue = (nValue * 16) + (c - '0');

				nEscaped++;
			}
			break;
		case 'a':
		case 'A':
		case 'b':
		case 'B':
		case 'c':
		case 'C':
		case 'd':
		case 'D':
		case 'e':
		case 'E':
		case 'f':
		case 'F':
			if (nEscaped >= 2 && nEscaped <= 5)
			{
				if (c >= 'a' && c <= 'f')
				{
					nValue = (nValue * 16) + (c - 'a' + 10);
				}
				else if (c >= 'A' && c <= 'F')
				{
					nValue = (nValue * 16) + (c - 'A' + 10);
				}

				nEscaped++;
			}
			break;
		}

		if (nEscaped == 0 && c == '\0')
		{
			break;
		}

		if (nOldEscaped == nEscaped || nEscaped == 6)
		{
			if (nEscaped > 1 && nEscaped < 6)
			{
				i--;
			}

			if (nEscaped <= 1)
			{
				nValue = c;
			}

			if (nIndex < nMaxValues)
			{
				values[nIndex] = nValue;
				nIndex++;
			}

			nOldEscaped = 0;
			nEscaped = 0;
			nValue = 0;
		}
	}

	return nIndex;
}

/**
 * Populate the key binding map for the Palm Pre.
 */
int TerminalConfigManager::generateDefaultKeyBindings()
{
	freeKeyBindings();

	for (int i = 0; i < TERM_KEYMOD_MAX; i++)
	{
		m_keyBindings[i] = new std::map<int, int>;
	}

	//Regular key.
	for (int i = 'a'; i <= 'z'; i++)
	{
		(*m_keyBindings[TERM_KEYMOD_NONE])[i] = i;
	}

	(*m_keyBindings[TERM_KEYMOD_NONE])['\x08'] = '\x08'; //Backspace
	(*m_keyBindings[TERM_KEYMOD_NONE])[','] = ',';
	(*m_keyBindings[TERM_KEYMOD_NONE])['.'] = '.';
	(*m_keyBindings[TERM_KEYMOD_NONE])['@'] = '@';
	(*m_keyBindings[TERM_KEYMOD_NONE])['\x20'] = '\x20'; //Space
	(*m_keyBindings[TERM_KEYMOD_NONE])['\x0D'] = '\x0D'; //Return

	//Shift key.
	for (int i = 'a'; i <= 'z'; i++)
	{
		(*m_keyBindings[TERM_KEYMOD_SHIFT])[i] = (i - 'a' + 'A');
	}

	(*m_keyBindings[TERM_KEYMOD_SHIFT])['\x08'] = '\x08'; //Backspace
	(*m_keyBindings[TERM_KEYMOD_SHIFT])[','] = ',';
	(*m_keyBindings[TERM_KEYMOD_SHIFT])['.'] = '.';
	(*m_keyBindings[TERM_KEYMOD_SHIFT])['@'] = '@';
	(*m_keyBindings[TERM_KEYMOD_SHIFT])['\x20'] = '\x20'; //Space
	(*m_keyBindings[TERM_KEYMOD_SHIFT])['\x0D'] = '\x0D'; //Return

	//FN key.

	//Alt key.
	(*m_keyBindings[TERM_KEYMOD_ALT])['a'] = '&';
	(*m_keyBindings[TERM_KEYMOD_ALT])['b'] = '#';
	(*m_keyBindings[TERM_KEYMOD_ALT])['c'] = '8';
	(*m_keyBindings[TERM_KEYMOD_ALT])['d'] = '4';
	(*m_keyBindings[TERM_KEYMOD_ALT])['e'] = '1';
	(*m_keyBindings[TERM_KEYMOD_ALT])['f'] = '5';
	(*m_keyBindings[TERM_KEYMOD_ALT])['g'] = '6';
	(*m_keyBindings[TERM_KEYMOD_ALT])['h'] = '$';
	(*m_keyBindings[TERM_KEYMOD_ALT])['i'] = '%';
	(*m_keyBindings[TERM_KEYMOD_ALT])['j'] = '!';
	(*m_keyBindings[TERM_KEYMOD_ALT])['k'] = ':';
	(*m_keyBindings[TERM_KEYMOD_ALT])['l'] = '\'';
	(*m_keyBindings[TERM_KEYMOD_ALT])['m'] = ';';
	(*m_keyBindings[TERM_KEYMOD_ALT])['n'] = '?';
	(*m_keyBindings[TERM_KEYMOD_ALT])['o'] = '"';
	(*m_keyBindings[TERM_KEYMOD_ALT])['p'] = '=';
	(*m_keyBindings[TERM_KEYMOD_ALT])['q'] = '/';
	(*m_keyBindings[TERM_KEYMOD_ALT])['r'] = '2';
	(*m_keyBindings[TERM_KEYMOD_ALT])['s'] = '-';
	(*m_keyBindings[TERM_KEYMOD_ALT])['t'] = '3';
	(*m_keyBindings[TERM_KEYMOD_ALT])['u'] = ')';
	(*m_keyBindings[TERM_KEYMOD_ALT])['v'] = '9';
	(*m_keyBindings[TERM_KEYMOD_ALT])['w'] = '+';
	(*m_keyBindings[TERM_KEYMOD_ALT])['x'] = '7';
	(*m_keyBindings[TERM_KEYMOD_ALT])['y'] = '(';
	(*m_keyBindings[TERM_KEYMOD_ALT])['z'] = '*';
	(*m_keyBindings[TERM_KEYMOD_ALT])['\x08'] = '\x08'; //Backspace
	(*m_keyBindings[TERM_KEYMOD_ALT])[','] = '_';
	(*m_keyBindings[TERM_KEYMOD_ALT])['.'] = '.';
	(*m_keyBindings[TERM_KEYMOD_ALT])['@'] = '0';
	(*m_keyBindings[TERM_KEYMOD_ALT])['\x20'] = '\x20'; //Space
	(*m_keyBindings[TERM_KEYMOD_ALT])['\x0D'] = '\x0D'; //Return

	//Ctrl key.
	for (int i = 'a'; i <= 'z'; i++)
	{
		(*m_keyBindings[TERM_KEYMOD_CTRL])[i] = (i - 'a' + 1);
	}

	(*m_keyBindings[TERM_KEYMOD_CTRL])['\x08'] = '\x08'; //Backspace
	(*m_keyBindings[TERM_KEYMOD_CTRL])[','] = ',';
	(*m_keyBindings[TERM_KEYMOD_CTRL])['.'] = '.';
	(*m_keyBindings[TERM_KEYMOD_CTRL])['@'] = '@';
	(*m_keyBindings[TERM_KEYMOD_CTRL])['\x20'] = '\x20'; //Space
	(*m_keyBindings[TERM_KEYMOD_CTRL])['\x0D'] = '\x0D'; //Return

	return 0;
}

int TerminalConfigManager::generateKeyBindings()
{
	char sKeysSection[] = "Keys";
	int values[TERM_KEYMOD_MAX];
	int key;
	int nNumValues;
	std::map<char *, char *, cmp_str> *map;
	std::map<char *, char *, cmp_str>::iterator itr;

	freeKeyBindings();
	map = getSection(sKeysSection);

	if (map == NULL)
	{
		generateDefaultKeyBindings();
	}
	else
	{
		for (itr = map->begin(); itr != map->end(); itr++)
		{
			memset(values, 0, sizeof(values));
			parseString(itr->first, &key, 1);
			nNumValues = parseString(itr->second, values, TERM_KEYMOD_MAX);

			for (int i = 0; i < TERM_KEYMOD_MAX; i++)
			{
				if (m_keyBindings[i] == NULL)
				{
					m_keyBindings[i] = new std::map<int, int>;
				}

				if (i >= nNumValues)
				{
					values[i] = key;
				}

				(*m_keyBindings[i])[key] = values[i];
			}
		}
	}

	return 0;
}
