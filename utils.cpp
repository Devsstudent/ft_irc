#include "utils.hpp"
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <string>
#include "client.hpp"
#include "channel.hpp"

void remove_backslash_r( std::string& c ) {
	size_t idx = c.find( '\r' );
	if ( idx != std::string::npos )
		c.erase( idx, 1 );
}

std::string trim( const std::string& s ) {
	std::string res;

	res = s;
	std::remove( res.begin(), res.end(), ' ' );
	return ( res );
}

std::string format( std::string format, ... ) {
	va_list va_args;
	size_t  i = 0;

	va_start( va_args, format );
	std::string s( format );
	while ( s.find( '%', i ) != std::string::npos ) {
		i = s.find( '%', i );
		if ( s[i + 1] == 's' )
			s.replace( i, 2, (char*) va_arg( va_args, char* ) );
		else if ( s[i + 1] == 'c' ) {
			s.replace( i, 1, 1, ' ' );
			s.replace( i + 1, 1, 1, (char) va_arg( va_args, int ) );
		} else if ( s[i + 1] == 'd' ) {
			std::stringstream ss;
			ss << (int) va_arg( va_args, int );
			s.replace( i, 2, ss.str() );
		} else
			break;
	}
	va_end( va_args );
	return ( s );
}

std::string format( std::string format, va_list va_args ) {
	size_t i = 0;

	std::string s = format;
	while ( s.find( '%', i ) != std::string::npos ) {
		i = s.find( '%', i );
		if ( s[i + 1] == 's' )
			s.replace( i, 2, (char*) va_arg( va_args, char* ) );
		else if ( s[i + 1] == 'c' ) {
			s.replace( i, 1, 1, ' ' );
			s.replace( i + 1, 1, 1, (char) va_arg( va_args, int ) );
		} else if ( s[i + 1] == 'd' ) {
			std::stringstream ss;
			ss << (int) va_arg( va_args, int );
			s.replace( i, 2, ss.str() );
		} else
			break;
	}
	return ( s );
}

bool isUser( std::string str ) {
	return ( str.find( '!' ) != std::string::npos ||
	         str.find( '@' ) != std::string::npos || std::isalpha( str[0] ) );
}

bool isChannel( std::string str ) {
	if ( str[0] == '#' )
		return ( true );
	return ( false );
}

bool hasMode( Client& user, char mode ) {
	return ( user.getModes().find( mode ) != std::string::npos );
}

bool hasMode( Channel& user, char mode ) {
	return ( user.getModes().find( mode ) != std::string::npos );
}
