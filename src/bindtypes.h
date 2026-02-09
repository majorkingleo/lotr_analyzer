#pragma once

#include <dbi.h>

using namespace Tools;

const unsigned USER_NAME_LEN=200;
const unsigned SUBJECT_LEN=200;
const unsigned BODY_LEN=2000;
const unsigned FILE_LEN=1024;

const unsigned TIME_LEN=19;
const unsigned NAME_LEN=20;
const unsigned MAC_ADDRESS_LEN=17;
const unsigned IP_ADDRESS_LEN=15;
const unsigned ACTION_LEN=50;
const unsigned CONFIG_KEY_LEN=50;
const unsigned CONFIG_VALUE_LEN=FILE_LEN;
const unsigned SERMON_ACTION_LEN = 1024;
const unsigned SERMON_REACTION_LEN = 1024;

struct JANEIN
{
  enum ETYPE
	{
	  FIRST__=-1,
	  NEIN=0,
	  JA=1,
	  LAST__
	};
  static const std::string STYPES[];
};

template< class T > std::ostream & operator<<( std::ostream & out, const Tools::EnumRange<T> & jn )
{
  if( jn.value <= jn.FIRST__ || jn.value >= jn.LAST__ )
	return out << jn.STYPES[0];
  else
	return out << jn.STYPES[jn.value];
}

template< class T > std::istream & operator>>( std::istream & in, Tools::EnumRange<T> & jn )
{
  std::string s;

  in >> s;

  for( int i = jn.FIRST__ + 1; i < jn.LAST__; i++ )
	{
	  if( s == jn.STYPES[i] ) 
		{
		  jn.value = static_cast<JANEIN::ETYPE>(i);
		  break;
		}
	}
  return in;
}


template<class T> bool is_bool( const Tools::EnumRange<T> & t ) { return false; }

class TableType
{
 public:
  enum Type
	{
	  MYISAM,
	  INNODB
	};

  Type table_type;

 public:
  TableType( Type type = INNODB )
	: table_type( type )
	{}
  virtual ~TableType() {}
};

class Forkey
{
 public:
  const std::string own_table;
  const std::string field;
  const std::string target_table;
  const std::string target_field;
  
  Forkey( DBBindType *base, 
		  DBType *field_,
		  const std::string & target_table_,
		  const std::string & target_field_ );		  

  virtual ~Forkey() {}
};

class Forkeys
{
 public:
  std::vector< Ref<Forkey> > forkeys;

  void add_key( Forkey *key )
	{
	  forkeys.push_back( key );
	}
};

class BASE : public DBBindType, public TableType, public Forkeys
{
public:
  enum class HIST_TYPE {
	HIST_AN,
	HIST_AE,
	HIST_LO
  };



 public:
  DBTypeInt      idx;
  DBTypeDateTime hist_an_zeit;
  DBTypeVarChar  hist_an_user;
  DBTypeDateTime hist_ae_zeit;
  DBTypeVarChar  hist_ae_user;  
  DBTypeDateTime hist_lo_zeit;
  DBTypeVarChar  hist_lo_user;  

  BASE( const std::string & name, DBBindType *base );

  BASE & operator=( const BASE & b )
	{ 
	  DBBindType::operator=(b); 
	  return *this;
	}

  void setHist( HIST_TYPE hist_type, const std::string & user = std::string() );
};

class CONFIG : public BASE
{
public:
	DBTypeVarChar	key;
	DBTypeVarChar	value;

public:
	CONFIG();
	CONFIG( const CONFIG & b )
	: CONFIG()
	{
		BASE::operator=(b);
	}

	CONFIG & operator=( const CONFIG & b )
	{
	  BASE::operator=(b);
	  return *this;
	}
};


class MAIL : public BASE
{
public:
	DBTypeVarChar		from;
	DBTypeVarChar		to;
	DBTypeVarChar		subject;
	DBTypeVarChar		body_text_plain;
	DBTypeVarChar		body_text_html;
	DBTypeVarChar		imap_filename;
	DBTypeInt 			checked;
	DBTypeInt 			found;
	DBTypeInt 			mailed;

public:
	MAIL();

	MAIL( const MAIL & b )
	: MAIL()
	{
		BASE::operator=(b);
	}
};

std::string create_sql( bool add_drop_table = false );

