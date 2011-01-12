
#include "mindtriscore/includes.h"
#include "database.h"

bool ServerDatabase::UserExists(string username)
{
	return users.find(username) != users.end();
}
	
bool ServerDatabase::UserMatchesPassword(string username, string password, unique_ptr<UserData> & userdata)
{
	map<string,unique_ptr<UserData>>::iterator it = users.find(username);
	if(it == users.end())
	{
		userdata.reset();
		return false;
	}
	else
	{
		userdata.reset(new UserData(*it->second));
		return it->second->GetPassword() == password;
	}
}

unique_ptr<UserData> ServerDatabase::FindUser(string username)
{
	map<string,unique_ptr<UserData>>::iterator it = users.find(username);
	if(it == users.end()) return nullptr;
	else return unique_ptr<UserData>(new UserData(*it->second));
}

void ServerDatabase::AddUser(string username, string displayname, string email, string password)
{
	users.insert(pair<string,UserData *>(username,new UserData(username,displayname,email,password)));
}
		
ServerDatabase::ServerDatabase(){

}

