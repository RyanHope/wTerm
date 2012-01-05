/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 * Copyright (C) 2011-2012 Ryan Hope <rmh3093@gmail.com>
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

#ifndef SDLCORE_HPP__
#define SDLCORE_HPP__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <map>

#include "terminal/vtterminalstate.hpp"

/**
 * Initializer and basic 2D function for webOS SDL.
 */
class SDLCore
{
protected:
	static const int BUFFER_DIRTY_BIT;
	static const int FONT_DIRTY_BIT;
	static const int FOREGROUND_COLOR_DIRTY_BIT;
	static const int BACKGROUND_COLOR_DIRTY_BIT;

	SDL_Surface *m_surface;
	SDL_Color m_backgroundColor;
	SDL_Color m_foregroundColor;

	int createFonts(int nSize);

	bool isDirty();
	bool isDirty(int nDirtyBits);
	void setDirty(int nDirtyBits);
	void clearDirty(int nDirtyBits);

	int getNextPowerOfTwo(int n);

	virtual int initCustom();
	virtual void handleKeyboardEvent(SDL_Event &event);
	virtual void handleMouseEvent(SDL_Event &event);

private:
	bool m_bRunning;

	int m_nWidth;
	int m_nHeight;

	int m_nDirtyBits;

	int m_nFontSize;

	TTF_Font *m_fontNormal;
	TTF_Font *m_fontBold;
	TTF_Font *m_fontItal;
	TTF_Font *m_fontBoldItal;
	int m_nFontHeight;
	int m_nFontWidth;
	int m_nMaxLinesOfText;
	int m_nMaxColumnsOfText;

	std::map<int, int> m_powerOfTwoLookup;

	int init();
	int initOpenGL();
	void shutdown();
	void eventLoop();

	void closeFonts();

public:
	SDLCore();
	virtual ~SDLCore();

	void start();
	void run();
	bool isRunning();

	void setResolution(int nWidth, int nHeight);

	int getFontSize();
	int setFontSize(int nSize);

	void printText(int nColumn, int nLine, const char *sText, bool bBold, bool bItalic);
	void drawCursor(int nColumn, int nLine);
	void drawRect(int nX, int nY, int nWidth, int nHeight, SDL_Color color, float fAlpha);
	void drawText(int nX, int nY, const char *sText, bool bBold, bool bItalic);
	void drawSurface(int nX, int nY, SDL_Surface *surface);
	void drawImage(int nX, int nY, const char *sImage);

	void setForegroundColor(unsigned char nRed, unsigned char nGreen, unsigned char nBlue);
	void setBackgroundColor(unsigned char nRed, unsigned char nGreen, unsigned char nBlue);
	void clearScreen();

	int getMaximumLinesOfText();
	int getMaximumColumnsOfText();

	virtual SDL_Color getColor(TSColor_t color);
	virtual void redraw();
	virtual void updateDisplaySize();
};

#endif
