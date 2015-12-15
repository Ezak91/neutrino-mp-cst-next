/*
 * lua misc functions
 *
 * (C) 2014-2015 M. Liebmann (micha-bbg)
 * (C) 2014 Sven Hoefer (svenhoefer)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <global.h>
#include <system/debug.h>
#include <gui/infoclock.h>
#include <cs_api.h>
#include <neutrino.h>

#include "luainstance.h"
#include "lua_misc.h"

CLuaInstMisc* CLuaInstMisc::getInstance()
{
	static CLuaInstMisc* LuaInstMisc = NULL;

	if(!LuaInstMisc)
		LuaInstMisc = new CLuaInstMisc();
	return LuaInstMisc;
}

CLuaMisc *CLuaInstMisc::MiscCheckData(lua_State *L, int n)
{
	return *(CLuaMisc **) luaL_checkudata(L, n, LUA_MISC_CLASSNAME);
}

void CLuaInstMisc::LuaMiscRegister(lua_State *L)
{
	luaL_Reg meth[] = {
		{ "new",             CLuaInstMisc::MiscNew },
		{ "strFind",         CLuaInstMisc::strFind },
		{ "strSub",          CLuaInstMisc::strSub },
		{ "enableInfoClock", CLuaInstMisc::enableInfoClock },
		{ "runScript",       CLuaInstMisc::runScriptExt },
		{ "GetRevision",     CLuaInstMisc::GetRevision },
		{ "checkVersion",    CLuaInstMisc::checkVersion },
		{ "postMsg",         CLuaInstMisc::postMsg },
		{ "__gc",            CLuaInstMisc::MiscDelete },
		{ NULL, NULL }
	};

	luaL_newmetatable(L, LUA_MISC_CLASSNAME);
	luaL_setfuncs(L, meth, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");
	lua_setglobal(L, LUA_MISC_CLASSNAME);
}

int CLuaInstMisc::MiscNew(lua_State *L)
{
	CLuaMisc **udata = (CLuaMisc **) lua_newuserdata(L, sizeof(CLuaMisc *));
	*udata = new CLuaMisc();
	luaL_getmetatable(L, LUA_MISC_CLASSNAME);
	lua_setmetatable(L, -2);
	return 1;
}

int CLuaInstMisc::strFind(lua_State *L)
{
	int numargs = lua_gettop(L);
	if (numargs < 3) {
		printf("CLuaInstMisc::%s: not enough arguments (%d, expected 2 (or 3 or 4))\n", __func__, numargs);
		lua_pushnil(L);
		return 1;
	}
	const char *s1;
	const char *s2;
	int pos=0, n=0, ret=0;
	s1 = luaL_checkstring(L, 2);
	s2 = luaL_checkstring(L, 3);
	if (numargs > 3)
		pos = luaL_checkint(L, 4);
	if (numargs > 4)
		n = luaL_checkint(L, 5);

	std::string str(s1);
	if (numargs > 4)
		ret = str.find(s2, pos, n);
	else
		ret = str.find(s2, pos);

	LUA_DEBUG("####[%s:%d] str_len: %d, s2: %s, pos: %d, n: %d, ret: %d\n", __func__, __LINE__, str.length(), s2, pos, n, ret);
	if (ret == (int)std::string::npos)
		lua_pushnil(L);
	else
		lua_pushinteger(L, ret);
	return 1;
}

int CLuaInstMisc::strSub(lua_State *L)
{
	int numargs = lua_gettop(L);
	if (numargs < 3) {
		printf("CLuaInstMisc::%s: not enough arguments (%d, expected 2 (or 3))\n", __func__, numargs);
		lua_pushstring(L, "");
		return 1;
	}
	const char *s1;
	size_t pos=0, len=std::string::npos;
	std::string ret="";
	s1 = luaL_checkstring(L, 2);
	pos = luaL_checkint(L, 3);
	if (numargs > 3)
		len = (size_t)luaL_checkint(L, 4);

	std::string str(s1);
	ret = str.substr(pos, len);

	LUA_DEBUG("####[%s:%d] str_len: %d, pos: %d, len: %d, ret_len: %d\n", __func__, __LINE__, str.length(), pos, len, ret.length());
	lua_pushstring(L, ret.c_str());
	return 1;
}

int CLuaInstMisc::enableInfoClock(lua_State *L)
{
	bool enable = true;
	int numargs = lua_gettop(L);
	if (numargs > 1)
		enable = _luaL_checkbool(L, 2);
	CInfoClock::getInstance()->enableInfoClock(enable);
	return 0;
}

int CLuaInstMisc::runScriptExt(lua_State *L)
{
	int numargs = lua_gettop(L);
	const char *script = luaL_checkstring(L, 2);
	std::vector<std::string> args;
	for (int i = 3; i <= numargs; i++) {
		std::string arg = luaL_checkstring(L, i);
		if (!arg.empty())
			args.push_back(arg);
	}

	CLuaInstance *lua = new CLuaInstance();
	lua->runScript(script, &args);
	args.clear();
	delete lua;
	return 0;
}

int CLuaInstMisc::GetRevision(lua_State *L)
{
	unsigned int rev = 0;
	std::string hw   = "";
#if HAVE_COOL_HARDWARE
	hw = "Coolstream";
#endif
	rev = cs_get_revision();
	lua_pushinteger(L, rev);
	lua_pushstring(L, hw.c_str());
	return 2;
}

int CLuaInstMisc::checkVersion(lua_State *L)
{
	int numargs = lua_gettop(L);
	if (numargs < 3) {
		printf("CLuaInstMisc::%s: not enough arguments (%d, expected 2)\n", __func__, numargs);
		lua_pushnil(L);
		return 1;
	}
	int major=0, minor=0;
	major = luaL_checkint(L, 2);
	minor = luaL_checkint(L, 3);
	if ((major > LUA_API_VERSION_MAJOR) || ((major == LUA_API_VERSION_MAJOR) && (minor > LUA_API_VERSION_MINOR))) {
		char msg[1024];
		snprintf(msg, sizeof(msg)-1, "%s (v%d.%d)\n%s v%d.%d",
				g_Locale->getText(LOCALE_LUA_VERSIONSCHECK1),
				LUA_API_VERSION_MAJOR, LUA_API_VERSION_MINOR,
				g_Locale->getText(LOCALE_LUA_VERSIONSCHECK2),
				major, minor);
		luaL_error(L, msg);
	}
	lua_pushinteger(L, 1); /* for backward compatibility */
	return 1;
}

int CLuaInstMisc::postMsg(lua_State *L)
{
	lua_Integer msg = 0;
	neutrino_msg_t post_msg = 0;
	msg = luaL_checkint(L, 2);
	switch (msg) {
		case POSTMSG_STANDBY_ON:
			post_msg = NeutrinoMessages::STANDBY_ON;
			break;
		default:
			return 0;
	}
	g_RCInput->postMsg(post_msg, 0);
	return 0;
}

int CLuaInstMisc::MiscDelete(lua_State *L)
{
	CLuaMisc *D = MiscCheckData(L, 1);
	delete D;
	return 0;
}

/* --------------------------------------------------------------
  deprecated functions
  --------------------------------------------------------------- */

//#define MISC_FUNC_DEPRECATED miscFunctionDeprecated
#define MISC_FUNC_DEPRECATED(...)

void CLuaInstMisc::miscFunctionDeprecated(lua_State *L, std::string oldFunc)
{
	std::string of = std::string("n:") + oldFunc + "()";
	std::string nf = std::string("M = misc.new(); M:") + oldFunc + "()";
	functionDeprecated(L, of.c_str(), nf.c_str());
	printf("  [see also] \33[33m%s\33[0m\n", LUA_WIKI "/Kategorie:Lua:Neutrino-API:SonstigeFunktionen:de");
}

int CLuaInstMisc::strFind_old(lua_State *L)
{
	MISC_FUNC_DEPRECATED(L, "strFind");
	return strFind(L);
}

int CLuaInstMisc::strSub_old(lua_State *L)
{
	MISC_FUNC_DEPRECATED(L, "strSub");
	return strSub(L);
}

int CLuaInstMisc::enableInfoClock_old(lua_State *L)
{
	MISC_FUNC_DEPRECATED(L, "enableInfoClock");
	return enableInfoClock(L);
}

int CLuaInstMisc::runScriptExt_old(lua_State *L)
{
	MISC_FUNC_DEPRECATED(L, "runScript");
	return runScriptExt(L);
}

int CLuaInstMisc::GetRevision_old(lua_State *L)
{
	MISC_FUNC_DEPRECATED(L, "GetRevision");
	return GetRevision(L);
}

int CLuaInstMisc::checkVersion_old(lua_State *L)
{
	MISC_FUNC_DEPRECATED(L, "checkVersion");
	return checkVersion(L);
}
