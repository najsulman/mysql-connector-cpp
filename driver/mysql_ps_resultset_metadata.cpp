/*
   Copyright (C) 2007 - 2008 MySQL AB, 2008 - 2009 Sun Microsystems, Inc.

   The MySQL Connector/C++ is licensed under the terms of the GPL
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPL as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
*/

#ifndef _WIN32
#include <stdlib.h>
#endif	//	_WIN32
#include <string.h>

#include <cppconn/exception.h>
#include "mysql_ps_resultset.h"
#include "mysql_ps_resultset_metadata.h"

#include "mysql_debug.h"
#include "mysql_util.h"

#define MAX_LEN_PER_CHAR 4

namespace sql
{
namespace mysql
{


/* {{{ MySQL_Prepared_ResultSetMetaData::MySQL_Prepared_ResultSetMetaData -I- */
MySQL_Prepared_ResultSetMetaData::MySQL_Prepared_ResultSetMetaData(MYSQL_STMT * s, sql::mysql::util::my_shared_ptr< MySQL_DebugLogger> * l)
  :logger(l? l->getReference():NULL)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::MySQL_Prepared_ResultSetMetaData");
	result_meta = mysql_stmt_result_metadata(s);
	num_fields = mysql_stmt_field_count(s);
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::~MySQL_Prepared_ResultSetMetaData -I- */
MySQL_Prepared_ResultSetMetaData::~MySQL_Prepared_ResultSetMetaData()
{
	/* Don't remove the block or we can get into problems with logger */
	{
		CPP_ENTER("MySQL_Prepared_ResultSetMetaData::~MySQL_Prepared_ResultSetMetaData");
		CPP_INFO_FMT("this=%p", this);
		mysql_free_result(result_meta);
	}
	logger->freeReference();
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getCatalogName -I- */
std::string
MySQL_Prepared_ResultSetMetaData::getCatalogName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getCatalogName");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return "";
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getColumnCount -I- */
unsigned int
MySQL_Prepared_ResultSetMetaData::getColumnCount()
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getColumnCount");
	CPP_INFO_FMT("this=%p", this);
	return num_fields;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getColumnDisplaySize -I- */
unsigned int
MySQL_Prepared_ResultSetMetaData::getColumnDisplaySize(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getColumnDisplaySize");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	int ret = mysql_fetch_field_direct(result_meta, columnIndex)->length;
	CPP_INFO_FMT("column=%u display_size=%d", columnIndex + 1, ret);
	return ret;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getColumnLabel -I- */
std::string
MySQL_Prepared_ResultSetMetaData::getColumnLabel(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getColumnLabel");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return mysql_fetch_field_direct(result_meta, columnIndex)->name;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getColumnName -I- */
std::string
MySQL_Prepared_ResultSetMetaData::getColumnName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getColumnName");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return mysql_fetch_field_direct(result_meta, columnIndex)->name;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getColumnType -I- */
int
MySQL_Prepared_ResultSetMetaData::getColumnType(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getColumnType");
	CPP_INFO_FMT("this=%p", this);
	CPP_INFO_FMT("column=%u", columnIndex);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	int mysql_type = mysql_fetch_field_direct(result_meta, columnIndex)->type;
	CPP_INFO_FMT("type=%d", mysql_type);
	return sql::mysql::util::mysql_type_to_datatype(
			mysql_fetch_field_direct(result_meta, columnIndex)
		);
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getColumnTypeName -I- */
std::string
MySQL_Prepared_ResultSetMetaData::getColumnTypeName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getColumnTypeName");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex; /* Indexed from 1 */
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return sql::mysql::util::mysql_type_to_string(
				mysql_fetch_field_direct(result_meta, columnIndex)
			);
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getPrecision -I- */
unsigned int
MySQL_Prepared_ResultSetMetaData::getPrecision(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getPrecision");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	unsigned int scale = getScale(columnIndex + 1);
	unsigned int ret = mysql_fetch_field_direct(result_meta, columnIndex)->length;
	if (scale) {
		ret -= scale + 1;
	}
	CPP_INFO_FMT("column=%u precision=%d", columnIndex + 1, ret);
	return ret;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getScale -I- */
unsigned int
MySQL_Prepared_ResultSetMetaData::getScale(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getScale");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	unsigned int ret = mysql_fetch_field_direct(result_meta, columnIndex)->decimals;
	CPP_INFO_FMT("column=%u scale=%d", columnIndex + 1, ret);
	return ret;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getSchemaName -I- */
std::string
MySQL_Prepared_ResultSetMetaData::getSchemaName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getSchemaName");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	const char * const db = mysql_fetch_field_direct(result_meta, columnIndex)->db;
	return db ? db : "";
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::getTableName -I- */
std::string
MySQL_Prepared_ResultSetMetaData::getTableName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::getTableName");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return mysql_fetch_field_direct(result_meta, columnIndex)->org_table;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isAutoIncrement -I- */
bool
MySQL_Prepared_ResultSetMetaData::isAutoIncrement(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isAutoIncrement");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return (mysql_fetch_field_direct(result_meta, columnIndex)->flags & AUTO_INCREMENT_FLAG ) != 0;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isCaseSensitive -I- */
bool
MySQL_Prepared_ResultSetMetaData::isCaseSensitive(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isCaseSensitive");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	const MYSQL_FIELD * const field = mysql_fetch_field_direct(result_meta, columnIndex);
	if (field->flags & NUM_FLAG || field->type == MYSQL_TYPE_NEWDECIMAL || field->type == MYSQL_TYPE_DECIMAL) {
		return false;
	}
	const sql::mysql::util::OUR_CHARSET * const cs = sql::mysql::util::find_charset(field->charsetnr);
	return NULL == strstr(cs->collation, "_ci");
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isCurrency -I- */
bool
MySQL_Prepared_ResultSetMetaData::isCurrency(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isCurrency");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return false;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isDefinitelyWritable -I- */
bool
MySQL_Prepared_ResultSetMetaData::isDefinitelyWritable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isDefinitelyWritable");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return isWritable(columnIndex + 1);
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isNullable -I- */
int
MySQL_Prepared_ResultSetMetaData::isNullable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isNullable");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return mysql_fetch_field_direct(result_meta, columnIndex)->flags & NOT_NULL_FLAG? columnNoNulls : columnNullable;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isReadOnly -I- */
bool
MySQL_Prepared_ResultSetMetaData::isReadOnly(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isReadOnly");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	/* Seems for Views, where the value is generated DB is empty everything else is set */
	const char * const db = mysql_fetch_field_direct(result_meta, columnIndex)->db;
	return !(db && strlen(db));
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isSearchable -I- */
bool
MySQL_Prepared_ResultSetMetaData::isSearchable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isSearchable");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return true;
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isSigned -I- */
bool
MySQL_Prepared_ResultSetMetaData::isSigned(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isSigned");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	if (mysql_fetch_field_direct(result_meta, columnIndex)->type == FIELD_TYPE_YEAR) {
		return false;
	}
	return !(mysql_fetch_field_direct(result_meta, columnIndex)->flags & UNSIGNED_FLAG);
}
/* }}} */


/* {{{ MySQL_Prepared_ResultSetMetaData::isWritable -I- */
bool
MySQL_Prepared_ResultSetMetaData::isWritable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_Prepared_ResultSetMetaData::isWritable");
	CPP_INFO_FMT("this=%p", this);
	--columnIndex;
	if (columnIndex >= num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
	return !isReadOnly(columnIndex + 1);
}
/* }}} */


}; /* namespace mysql */
}; /* namespace sql */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
