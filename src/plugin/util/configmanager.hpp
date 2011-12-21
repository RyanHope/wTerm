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

#ifndef CONFIGMANAGER_HPP__
#define CONFIGMANAGER_HPP__

#include <map>
#include <stdio.h>

#include "strcmp.hpp"

class ConfigManager
{
protected:
	/**
	 * Map for storing section name to a map of key/value pairs.
	 */
	std::map<char *, std::map<char *, char *, cmp_str> *, cmp_str> m_configMap;

	void freeConfig();
	char *copyString(const char *s);

public:
	ConfigManager();
	virtual ~ConfigManager();

	virtual int parse(const char *sFileName);
	void addValue(char *sSection, char *sKey, char *sValue);
	std::map<char *, char *, cmp_str> *getSection(char *sSection);
};

#endif
