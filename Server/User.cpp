#include <iostream>
#include "User.h"

using namespace std;


User::User(
	const int id,
	const string& name,
	const string& login,
	const SHA1PwdArray& pass_sha1_hash,
	const bool locked,
	const bool logged)
	:
	_id(id),
	_name(name),
	_login(login),
	_pass_sha1_hash(pass_sha1_hash),
	_locked(locked),
	_logged(logged)
{}

