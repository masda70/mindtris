#include "mt_server.h"

bool ServerDatabase::UserExists(string username)
{
	return users.find(username) != users.end();
}
	
bool ServerDatabase::UserMatchesPassword(string username, string password, UserData * &userdata)
{
	map<string,UserData *>::iterator it = users.find(username);
	if(it == users.end())
	{
		userdata = NULL;
		return false;
	}
	else
	{
		userdata = (it->second);
		return it->second->GetPassword() == password;
	}
}

UserData * ServerDatabase::FindUser(string username)
{
	map<string,UserData *>::iterator it = users.find(username);
	if(it == users.end()) return NULL;
	else return (it->second);
}

void ServerDatabase::AddUser(string username, string displayname, string email, string password)
{
	users.insert(pair<string,UserData *>(username,new UserData(username,displayname,email,password)));
}
		
ServerDatabase::ServerDatabase(){

}

