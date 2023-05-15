#include <list>
#include "channel.hpp"
#include "utils.hpp"

void part( std::list<std::string>* args, Client& c ) {
	std::string target = args->front();
	
	if ( isChannel( target ) ) {
		Channel* channel = find_channel( target );
		if ( channel ) {
			if (channel->findClients(c.getNick()))
			{
				if ( args->size() == 1 )
				{
				 channel->sendAll(
				  	format( ":%s!%s@%s PART %s\r\n", c.getNick().c_str(),
				       	c.getUser().c_str(), c.getHostname().c_str(),
				       	target.c_str() ) );
				}
				else
				{
				 channel->sendAll(
				    	format( ":%s!%s@%s PART %s :%s\r\n", c.getNick().c_str(),
				            	c.getUser().c_str(), c.getHostname().c_str(),
				            	target.c_str(), args->back().c_str() ) );
				channel->removeClient( c );
				}
			}
			else
				c.reply(format(":ircserv.locahost 442 %s :You're not on that channel\r\n", target.c_str()));
		}
		else
				c.reply(format(":ircserv.locahost 403 %s :No such channel\r\n", target.c_str()));
			channel->removeClient( c );
	}
}
