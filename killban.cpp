#include <list>
#include "client.hpp"
#include "utils.hpp"

// TODO: add RPLs and make some verification about how this command work !

void killban( std::list<std::string>* args, Client& c ) {
	if ( !c.isOper() )
		return;

	std::string target_name = args->front();
	if ( !isUser( target_name ) )
		return;
	Client* target = find_client( target_name );
	if ( !target )
		return;
	if ( target->isOper() )
		return;
	// ban part here
	delete target;
}
