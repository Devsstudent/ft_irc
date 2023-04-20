#include <unistd.h>
#include <iostream>
#include <list>
#include "client.hpp"
#include "irc.hpp"
#include "ircserv.hpp"
#include "utils.hpp"

#define COMMAND_COUNT 11

void privmsg( std::list<std::string>* args, Client *c );
void nick( std::list<std::string>* args, Client *c );
void user( std::list<std::string>* args, Client *c );
void whois( std::list<std::string>* args, Client *c );
void quit( std::list<std::string>* args, Client *c );
void join( std::list<std::string>* args, Client *c);

void pass( std::list<std::string>* args, Client *c ) {
	logger( "DEBUG", "PASS COMMAND" );
	if ( args->front().compare( ircserv::getPassword() ) != 0 ) {
		logger( "ERROR", "client %d : wrong password (%s)!", c->getFd(),
		        args->front().c_str() );
		close( c->getFd() );
		return;
	}
	c->setHasGivenPassword( true );
}


void capls( std::list<std::string>* args, Client *c) {
	std::cout << args->front() << " " << args->back() << std::endl;
	(void) c;
	(void) args;
}

std::ostream& operator<<( std::ostream& os, std::list<std::string> arg ) {
	std::list<std::string>::iterator it = arg.begin();
	while ( it != arg.end() ) {
		os << *it;
		it++;
	}
	return os;
}

void pong( std::list<std::string>* args, Client *c) {
	(void) args;
	args->front().erase( args->back().length() - 1, 1 );
	logger( "DEBUG", "PING from %s, token = %s", c->getNick().c_str(),
	        args->back().c_str() );
	c->reply( format( "PONG\r\n" ) );
}

/*IRC MODS:
 *
 * client : ioRZBT
 * server : imRMsuU (+ defaults : nt, +specials : bfkl, +roles:qoahv)
 *
 * go see documentation on notion !
 *
 */

static bool is_valid_user_mode( char mode ) {
	return ( mode == 'i' || mode == 'o' || mode == 'R' || mode == 'Z' ||
	         mode == 'B' || mode == 'T' );
}

static bool is_valid_channel_mode( char mode ) {
	return ( mode == 'i' || mode == 'm' || mode == 'R' || mode == 'M' ||
	         mode == 's' || mode == 'u' || mode == 'U' || mode == 'n' ||
	         mode == 't' || mode == 'b' || mode == 'f' || mode == 'k' ||
	         mode == 'l' || mode == 'q' || mode == 'a' || mode == 'h' ||
			 mode == 'v');
}

static std::string check_user_modes( std::string modes ) {
	std::string ret;

	if ( modes[0] == '+' )
		ret += '+';
	else if ( modes[0] == '-' )
		ret += '-';
	else
		return ( ret );
	for ( size_t i = 1; i < modes.length(); i++ ) {
		if ( is_valid_user_mode( modes[i] ) ) {
			ret += modes[i];
		}
	}
	return ret;
}

static std::string check_channel_modes( std::string modes ) {
	std::string ret;

	if ( modes[0] == '+' )
		ret += '+';
	else if ( modes[0] == '-' )
		ret += '-';
	else
		return ( ret );
	for ( size_t i = 1; i < modes.length(); i++ ) {
		if ( is_valid_channel_mode( modes[i] ) ) {
			ret += modes[i];
		}
	}
	return ret;
}

void user_mode( Client     *c,
                std::string target,
                std::string modes,
                char        operation ) {
	if ( modes.empty() ) {
		c->reply( format( ":ircserv.localhost 501 %s :Unknown MODE flag\r\n",
		                 c->getNick().c_str() ) );
		return;
	}
	if ( target != c->getNick() ) {
		c->reply( ":ircserv.localhost 502 :Cant change mode for other users" );
		return;
	}
	if ( operation == '+' )
		modes = c->addModes( modes );
	else if ( operation == '-' )
		modes = c->removeModes( modes );
	logger( "DEBUG", "user %s has now mode %s", c->getUser().c_str(),
	        modes.c_str() );
	c->reply( format( ":ircserv.localhost 221 %s %s\r\n", c->getUser().c_str(),
	                 modes.c_str() ) );
}

void channel_mode( Client     *c,
                   std::string target,
                   std::string modes,
                   char        operation ) {
	if ( modes.empty() ) {
		c->reply( format( ":ircserv.localhost 324 %s %s +\r\n",
		                 c->getUser().c_str(), target.c_str() ) );
		return;
	}
	try {
		ircserv::getChannels().at( target );
	} catch ( std::exception& e ) {
		c->reply( format( ":ircserv.localhost 403 %s :No such channel",
		                 target.c_str() ) );
	}
	logger( "DEBUG", "channel %s has now mode %s", target.c_str(),
	        modes.c_str() );
	c->reply( format( ":ircserv.localhost 324 %s %s +%s\r\n",
	                 c->getUser().c_str(), target.c_str(), modes.c_str() ) );
	(void) operation;
}

void mode( std::list<std::string>* args, Client *c) {
	if ( args->empty() )
		return;
	args->back().erase( args->back().length() - 1, 1 );
	std::string modes;
	char        operation = modes[0];
	modes.erase( 0, 1 );
	std::string target = args->front();
	logger( "DEBUG", "target = %s", target.c_str() );
	if ( isUser( target ) ) {
		modes = check_user_modes( args->back() );
		user_mode( c, target, modes, operation );
	}
	if ( isChannel( target ) ) {
		modes = check_channel_modes( args->back() );
		channel_mode( c, target, modes, operation );
	}
}

void handler( std::list<std::string>* args, Client *c ) {
	if ( args->size() == 1 ) {
		c->reply( format( ":ircserv.localhost 461 %s :Not enough parameters",
		                 args->front().c_str() ) );
		return;
	}
	std::string commands[COMMAND_COUNT] = { "PASS",    "USER",  "NICK", "JOIN",
	                                        "PRIVMSG", "CAPLS", "CAP",  "PING",
	                                        "MODE",    "WHOIS", "QUIT" };
	void ( *handlers[COMMAND_COUNT] )( std::list<std::string>*, Client *c ) = {
	    &pass,  &user, &nick, &join,  &privmsg, &capls,
	    &capls, &pong, &mode, &whois, &quit };
	for ( size_t i = 0; i < COMMAND_COUNT; i++ ) {
		//	std::cout << args->front() << std::endl;
		if ( !args->front().compare( commands[i] ) ) {
			args->pop_front();
			remove_backslash_r( args->back() );
			handlers[i]( args, c );
			return;
		}
	}
	logger( "WARNING", "%s COMMAND DO NOT EXIST (YET?)",
	        args->front().c_str() );
}
