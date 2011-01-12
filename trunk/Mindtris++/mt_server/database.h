#ifndef MT_USERDATA_H
#define MT_USERDATA_H

class UserData
{
private:
	string m_username;
	string m_displayname;
	string m_email;
	string m_password;
public:
	string GetUsername(){return m_username;}
	string GetDisplayName(){return m_displayname;}
	string GetEmail(){return m_email;}
	string GetPassword(){return m_password;}
	UserData(string username,string displayname, string email, string password){ m_username = username; m_displayname = displayname; m_email = email; m_password = password;}
};



class ServerDatabase
{
private:
	map<string,unique_ptr<UserData>> users;

public:

	bool UserExists(string username);	
	unique_ptr<UserData> FindUser(string username);
	bool UserMatchesPassword(string username, string password, unique_ptr<UserData> & userdata);
	void AddUser(string username, string displayname, string email, string password);
		
	ServerDatabase();

};

#endif // USERDATA_H