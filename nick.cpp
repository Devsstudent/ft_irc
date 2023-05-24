// Function to check if the nick is already in the db or not
// If it's the case return 433 "<client> <nick> :Nickname is already in use"

// We can define our accepted character list, if not corresping,
// return 432 "<client> <nick> :Erroneus nickname"

#include <unistd.h>
#include <iostream>
#include <list>
#include "client.hpp"
#include "irc.hpp"
#include "ircserv.hpp"
#include "messageBuilder.hpp"
#include "utils.hpp"

bool authorize_setting_name( const std::string& name, Client& c ) {
	for ( t_client_array::iterator it = ircserv::getClients().begin();
	      it != ircserv::getClients().end(); it++ ) {
		if ( it->getNick() == name && it->getFd() != c.getFd() )
			return ( false );
	}
	return ( true );
}

void nick( std::list<std::string>* args, Client& c ) {
	std::string    buff;
	MessageBuilder mb;
	// for ( std::list<std::string>::iterator it = args->begin();
	//       it != args->end(); it++ ) {
	// 	std::cout << "sent :" << *it << std::endl;
	// }
	if ( args->empty() || args->front() == "" ) {
		c.reply( mb << ":" << ircserv::getServername()
		            << " 431 * NICK :Not enough parameters\r\n" );
	} else if ( !authorize_setting_name( args->front(), c ) &&
	            c.isRegistered() ) {
		logger( "INFO", mb << "User " << c.getUser()
		                   << " tried to change nickname to " << args->front()
		                   << " but it's already taken." );

		c.reply( mb << ":" << ircserv::getServername() << " 433 * "
		            << c.getNick() << " :Nickname is already in use\r\n" );
	} else if ( c.hasGivenNick() ) {
		buff = c.getNick();
		c.setNick( args->front() );
		c.reply( mb << ":" << buff << "!~" << c.getUser() << "@"
		            << c.getHostname() << " NICK " << c.getNick() << "\r\n" );

		logger( "INFO", mb << "User " << c.getUser() << " nickname change to "
		                   << c.getNick() << "." );
	} else {
		buff = c.getNick();
		c.setNick( args->front() );
		c.reply( mb << ':' << buff << "!" << c.getUser() << "@"
		            << c.getHostname() << " NICK " << c.getNick() << "\r\n" );
		c.setHasGivenNick( true );

		logger( "INFO", mb << "User " << c.getUser() << " nickname change to "
		                   << c.getNick() << "." );
	}
}

void nick_notregistered( std::list<std::string>* args, Client& c ) {
	std::string buff = c.getNick();
	c.setNick( args->front() );
	c.setHasGivenNick( true );
}
