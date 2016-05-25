/*
	ni_menu

	(C) 2009-2016 NG-Team
	(C) 2016 NI-Team
	(C) 2016 TangoCash

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __lcd4l_setup__
#define __lcd4l_setup__

#include <gui/widget/menue.h>

#define FLAG_DIR		"/var/etc/"

using namespace std;

class CNITouchFileNotifier : public CChangeObserver
{
	const char * filename;
	public:
		inline CNITouchFileNotifier(const char * file_name) { filename = file_name; };
		bool changeNotify(const neutrino_locale_t, void * data);
};

class CLCD4Linux_Setup : public CMenuTarget, CChangeObserver
{
	private:
		CMenuOptionChooser *mc;
		CMenuForwarder * mf;

		int width;
		int show();

	public:
		CLCD4Linux_Setup();
		~CLCD4Linux_Setup();
		static CLCD4Linux_Setup* getInstance();
		int exec(CMenuTarget* parent, const std::string &actionkey);
		virtual bool changeNotify(const neutrino_locale_t OptionName, void * /*data*/);
};

#endif