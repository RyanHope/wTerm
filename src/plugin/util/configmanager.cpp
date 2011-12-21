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

#include "configmanager.hpp"
#include "logger.hpp"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
	freeConfig();
}

void ConfigManager::freeConfig()
{
	for (std::map<char *, std::map<char *, char *, cmp_str> *, cmp_str>::iterator itr = m_configMap.begin(); itr != m_configMap.end(); itr++)
	{
		std::map<char *, char *, cmp_str> *map = itr->second;

		for (std::map<char *, char *, cmp_str>::iterator itr2 = map->begin(); itr2 != map->end(); itr2++)
		{
			free(itr2->first);
			free(itr2->second);
		}

		free(itr->first);
		delete map;
	}

	m_configMap.clear();
}

/**
 * Creates a copy of a string. The caller must free the memory.
 * If the source is NULL, then an empty string is allocated.
 */
char *ConfigManager::copyString(const char *s)
{
	char *newString;

	if (s == NULL)
	{
		newString = (char *)malloc(sizeof(char));
		memcpy(newString, "", sizeof(char));
	}
	else
	{
		size_t size = (strlen(s) + 1) * sizeof(char);
		newString = (char *)malloc(size);
		memcpy(newString, s, size);
	}

	return newString;
}

void ConfigManager::addValue(char *sSection, char *sKey, char *sValue)
{
	std::map<char *, char *, cmp_str> *sectionMap = getSection(sSection);
	std::map<char *, char *, cmp_str>::iterator locator;

	if (sectionMap == NULL)
	{
		//Create a section.
		m_configMap[copyString(sSection)] = new std::map<char *, char *, cmp_str>;
	}

	sectionMap = getSection(sSection);
	locator = sectionMap->find(sKey);

	if (locator != sectionMap->end())
	{
		//Delete old value.
		free(locator->first);
		free(locator->second);

		sectionMap->erase(locator);
	}

	(*sectionMap)[copyString(sKey)] = copyString(sValue);
}

int ConfigManager::parse(const char *sFileName)
{
	FILE *file;
	char buffer[512];
	char lastSection[512];
	char value[512];
	char key[512];
	int nIndex, nEndIndex;
	int nResult = 0;

	Logger::getInstance()->dump("Reading configuration file: '%s'", sFileName);

	file = fopen(sFileName, "r");

	if (file == NULL)
	{
		Logger::getInstance()->error("Cannot read configuration file: '%s'", sFileName);
		return -1;
	}

	memset(lastSection, 0, sizeof(lastSection));
	freeConfig();

	while (fgets(buffer, sizeof(buffer), file) != NULL)
	{
		Logger::getInstance()->dump("Processing configuration line: '%s'", buffer);

		nIndex = 0;

		while (buffer[nIndex] != '\0' && isspace(buffer[nIndex]))
		{
			nIndex++;
		}

		nEndIndex = nIndex;

		if (buffer[nIndex] == ';' || buffer[nIndex] == '#')
		{
			//Skip comments.
			continue;
		}
		else if (buffer[nIndex] == '[')
		{
			//Parse section.
			nIndex++;
			nEndIndex = nIndex;

			while (buffer[nEndIndex] != '\0' && buffer[nEndIndex] != ']')
			{
				nEndIndex++;
			}

			if (buffer[nEndIndex] != ']')
			{
				Logger::getInstance()->error("Reading configuration section.");
				nResult = -1;
				break;
			}

			nEndIndex--;

			if (nEndIndex >= nIndex)
			{
				memcpy(lastSection, buffer + nIndex, (nEndIndex - nIndex + 1) * sizeof(char));
			}
			else
			{
				memset(lastSection, 0, sizeof(lastSection));
			}

			Logger::getInstance()->dump("Setting current Section: '%s'", lastSection);
		}
		else
		{
			memset(key, 0, sizeof(key));
			memset(value, 0, sizeof(value));

			while (buffer[nEndIndex] != '\0' && buffer[nEndIndex] != '=')
			{
				nEndIndex++;
			}

			if (buffer[nEndIndex] != '=' || nIndex >= nEndIndex)
			{
				Logger::getInstance()->error("Reading configuration entry.");
				continue;
			}

			//Store key.
			memcpy(key, buffer + nIndex, (nEndIndex - nIndex) * sizeof(char));

			//Store value.
			nIndex = nEndIndex + 1;
			nEndIndex = strlen(buffer) - 1;

			//Trim whitespaces
			while (isspace(buffer[nEndIndex]))
			{
				nEndIndex--;
			}

			if (nIndex <= nEndIndex)
			{
				memcpy(value, buffer + nIndex, (nEndIndex - nIndex + 1) * sizeof(char));
			}

			Logger::getInstance()->dump("Adding configuration entry: Section '%s', Key '%s', Value '%s'", lastSection, key, value);

			addValue(lastSection, key, value);
		}
	}

	fclose(file);

	//Invalidate config.
	if (nResult != 0)
	{
		freeConfig();
	}

	return nResult;
}

std::map<char *, char *, cmp_str> *ConfigManager::getSection(char *sSection)
{
	std::map<char *, std::map<char *, char *, cmp_str> *, cmp_str>::iterator locator = m_configMap.find(sSection);

	if (locator == m_configMap.end())
	{
		return NULL;
	}

	return locator->second;
}
