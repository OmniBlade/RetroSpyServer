/*
    This file is part of RetroSpy Server.

    RetroSpy Server is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Helper.h"

#include <MDK/Query.h>

#include <stdio.h>
#include <string.h>
#include <limits.h>

bool ExecuteFirstQuery(CDatabase* db, CResultSet *res, std::string query)
{
	if (!res->ExecuteQuery(db, query))
		return false;

	if (!res->GotoFirstRow())
		return false;

	return true;
}

unsigned int GetProfileIDFromNickEmail(CDatabase* db, const char *nick, const char *email)
{
	std::string query = "";
	std::string _email = email, _nick = nick;
	CResultSet *res = new CResultSet();
	unsigned int ret = 0;

	query = "SELECT profiles.profileid FROM profiles INNER JOIN"
			" users ON profiles.userid=users.userid WHERE"
			" users.email='";
	if (!mdk_escape_query_string(db, _email))
		return false;
	query += _email;
	query += "' AND profiles.nick='";
	if (!mdk_escape_query_string(db, _nick))
		return false;
	query += _nick;
	query += "'";

	if (!ExecuteFirstQuery(db, res, query))
	{
		delete res;
		return ret;
	}

	ret = res->GetUIntFromRow(0);

	delete res;
	return ret;
}

void GetUniqueNickFromProfileID(CDatabase* db, unsigned int pid, char *unick, int size)
{
	std::string query = "";
	CResultSet *res = new CResultSet();

	query = "SELECT uniquenick FROM profiles WHERE profileid='";
	query += std::to_string(pid);
	query += "'";

	if (!ExecuteFirstQuery(db, res, query))
	{
		delete res;
		return;
	}

	strncpy_s(unick, size, res->GetStringFromRow(0).c_str(), size - 1);

	delete res;
}

bool GetProfileIDFromUniqueNick(CDatabase* db, const char *unick, unsigned int* out)
{
	std::string query = "", _unick = unick;
	CResultSet *res = new CResultSet();

	query = "SELECT profileid FROM profiles WHERE uniquenick='";
	if (!mdk_escape_query_string(db, _unick))
	{
		delete res;
		return false;
	}
	query += _unick;
	query += "'";

	if (!ExecuteFirstQuery(db, res, query))
	{
		delete res;
		return false;
	}

	*out = res->GetUIntFromRow(0);

	delete res;
	return true;
}

void GetPasswordFromUserID(CDatabase* db, char *out, int out_len, unsigned int id)
{
	CResultSet *res = new CResultSet();
	char query[61];
	query[0] = 0;

	_snprintf_s(query, sizeof(query), sizeof(query) - 1, "SELECT password FROM users WHERE userid='%u'", id);

	if (!ExecuteFirstQuery(db, res, query))
	{
		delete res;
		return;
	}

	strncpy_s(out, out_len, res->GetStringFromRow(0).c_str(), out_len - 1);

	delete res;
}

unsigned int GetUserIDFromProfileID(CDatabase* db, unsigned int id, unsigned int* out)
{
	CResultSet *res = new CResultSet();
	char query[61];
	unsigned int ret = 0;
	query[0] = 0;

	_snprintf_s(query, sizeof(query), sizeof(query) - 1, "SELECT userid FROM profiles WHERE profileid='%u'", id);

	if (!ExecuteFirstQuery(db, res, query))
	{
		delete res;
		return false;
	}

	ret = res->GetUIntFromRow(0);
	*out = ret;

	delete res;
	return true;
}

int AssignSessionKeyFromProfileID(CDatabase* db, unsigned int profileid)
{
	int ssk = 0;
	bool bx = true;
	char query[61];
	query[0] = 0;

	while (bx)
	{
		ssk = RandInt(1, INT_MAX);
		_snprintf_s(query, sizeof(query), sizeof(query) - 1, "UPDATE profiles SET sesskey=%d WHERE profileid='%u'", ssk, profileid);

		if (mdk_only_run_query(db, query))
			bx = false;
	}

	return ssk;
}

void FreeSessionKey(CDatabase* db, unsigned int profileid)
{
	char query[61];
	query[0] = 0;

	_snprintf_s(query, sizeof(query), sizeof(query) - 1, "UPDATE profiles SET sesskey=NULL WHERE profileid='%u'", profileid);

	mdk_only_run_query(db, query);
}

int GetPublicMaskFromProfileID(CDatabase* db, unsigned int pid)
{
	CResultSet *res = new CResultSet();
	std::string query = "";
	int ret = 0;

	query = "SELECT publicmask FROM profiles WHERE profileid=";
	query += std::to_string(pid);

	if (!ExecuteFirstQuery(db, res, query))
	{
		delete res;
		return ret;
	}

	ret = res->GetIntFromRow(0);

	delete res;
	return ret;
}

bool GetProfileInfo(CDatabase* db, unsigned int pid, GPIInfoCache *out, unsigned int *id_out)
{
#define resget(x) res->GetStringFromRow(x).c_str()

	CResultSet *res = new CResultSet();
	std::string query = "";

	query = "SELECT profiles.uniquenick, profiles.nick, profiles.firstname, profiles.lastname, profiles.latitude," //5
		" profiles.longitude, profiles.publicmask, profiles.userid, profiles.aim, profiles.picture," //9
		" profiles.occupationid, profiles.incomeid, profiles.industryid, profiles.marriedid, profiles.childcount," //14
		" profiles.interests1, profiles.ownership1, profiles.connectiontype, profiles.sex, profiles.zipcode," //19
		" profiles.countrycode, profiles.homepage, profiles.birthday, profiles.birthmonth, profiles.birthyear," //24
		" profiles.location, profiles.icq, users.email FROM profiles INNER JOIN users ON users.userid=profiles.userid WHERE profiles.profileid=";
	
	query += std::to_string(pid);

	if (!ExecuteFirstQuery(db, res, query))
	{
		delete res;
		return false;
	}

	strncpy_s(out->uniquenick, sizeof(out->uniquenick), resget(0), sizeof(out->uniquenick) - 1);
	strncpy_s(out->nick, sizeof(out->nick), resget(1), sizeof(out->nick) - 1);
	strncpy_s(out->firstname, sizeof(out->firstname), resget(2), sizeof(out->firstname) - 1);
	strncpy_s(out->lastname, sizeof(out->lastname), resget(3), sizeof(out->lastname) - 1);
	out->latitude = (float)res->GetDoubleFromRow(4);
	out->longitude = (float)res->GetDoubleFromRow(5);
	out->publicmask = res->GetIntFromRow(6);
	*id_out = res->GetUIntFromRow(7);
	strncpy_s(out->aimname, sizeof(out->aimname), resget(8), sizeof(out->aimname) - 1);
	out->pic = res->GetIntFromRow(9);
	out->occupationid = res->GetIntFromRow(10);
	out->incomeid = res->GetIntFromRow(11);
	out->industryid = res->GetIntFromRow(12);
	out->marriedid = res->GetIntFromRow(13);
	out->childcount = res->GetIntFromRow(14);
	out->interests1 = res->GetIntFromRow(15);
	out->ownership1 = res->GetIntFromRow(16);
	out->conntypeid = res->GetIntFromRow(17);

	if (res->GetStringFromRow(18).compare("MALE") == 0)
		out->sex = GP_MALE;
	else if (res->GetStringFromRow(18).compare("FEMALE") == 0)
		out->sex = GP_FEMALE;
	else
		out->sex = GP_PAT;	

	strncpy_s(out->zipcode, sizeof(out->zipcode), resget(19), sizeof(out->zipcode) - 1);
	strncpy_s(out->countrycode, sizeof(out->countrycode), resget(20), sizeof(out->countrycode) - 1);
	strncpy_s(out->homepage, sizeof(out->homepage), resget(21), sizeof(out->homepage) - 1);
	out->birthday = res->GetIntFromRow(22);
	out->birthmonth = res->GetIntFromRow(23);
	out->birthyear = res->GetIntFromRow(24);
	strncpy_s(out->place, sizeof(out->place), resget(25), sizeof(out->place) - 1);
	out->icquin = res->GetIntFromRow(26);
	strncpy_s(out->email, sizeof(out->email), resget(27), sizeof(out->email) - 1);

	delete res;

	return true;
}
