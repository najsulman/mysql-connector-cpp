/*
   Copyright 2007 - 2008 MySQL AB, 2008 - 2009 Sun Microsystems, Inc.  All rights reserved.

   The MySQL Connector/C++ is licensed under the terms of the GPL
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPL as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
*/


#include <cppconn/exception.h>
#include "mysql_util.h"
#include "mysql_debug.h"
#include "mysql_resultbind.h"
#include "mysql_client_api.h"
#include "mysql_resultset_data.h"

#include <string.h>

namespace sql
{
namespace mysql
{

struct st_buffer_size_type
{
	char * buffer;
	size_t size;
	enum_field_types type;
	st_buffer_size_type(char * b, size_t s, enum_field_types t) : buffer(b), size(s), type(t) {}
};


/* {{{ allocate_buffer_for_field() -I- */
typedef std::pair<char *, size_t> BufferSizePair;
static struct st_buffer_size_type
allocate_buffer_for_field(const MYSQL_FIELD * const field)
{
	switch (field->type) {
		case MYSQL_TYPE_NULL:
			return st_buffer_size_type(NULL, 0, field->type);
		case MYSQL_TYPE_TINY:
			return st_buffer_size_type(new char[1], 1, field->type);
		case MYSQL_TYPE_SHORT:
			return st_buffer_size_type(new char[2], 2, field->type);
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_FLOAT:
			return st_buffer_size_type(new char[4], 4, field->type);
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_LONGLONG:
			return st_buffer_size_type(new char[8], 8, field->type);
		case MYSQL_TYPE_YEAR:
			return st_buffer_size_type(new char[2], 2, MYSQL_TYPE_SHORT);
		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_DATE:
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_DATETIME:
			return st_buffer_size_type(new char[sizeof(MYSQL_TIME)], sizeof(MYSQL_TIME), field->type);

		
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
			return st_buffer_size_type(new char[field->max_length + 1], field->max_length + 1, field->type);

		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_NEWDECIMAL:
			return st_buffer_size_type(new char[64], 64, field->type);
#if A1
		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_YEAR:
			return st_buffer_size_type(new char[10], 10, field->type);
#endif
#if A0
		// There two are not sent over the wire
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
#endif
		case MYSQL_TYPE_BIT:
			return st_buffer_size_type(new char[8], 8, MYSQL_TYPE_LONGLONG);
		case MYSQL_TYPE_GEOMETRY:
		default:
                        // TODO: Andrey, there can be crashes when we go through this. Please fix.			
			throw sql::InvalidArgumentException("allocate_buffer_for_field: invalid rbind data type");
	}
}
/* }}} */


/* {{{ MySQL_ResultBind::MySQL_ResultBind -I- */
MySQL_ResultBind::MySQL_ResultBind(MYSQL_STMT * s, boost::shared_ptr< NativeAPI::IMySQLCAPI > & _capi,
                 boost::shared_ptr< MySQL_DebugLogger > & log)
                 : num_fields(0), is_null(NULL), err(NULL), len(NULL), capi(_capi), logger(log),
                 stmt(s), rbind(NULL)
{
}
/* }}} */


/* {{{ MySQL_ResultBind::~MySQL_ResultBind() -I- */
MySQL_ResultBind::~MySQL_ResultBind()
{
	if (rbind.get()) {
		for (unsigned int i = 0; i < num_fields; ++i) {
			delete[] (char *) rbind[i].buffer;
		}
	}
}
/* }}} */


/* {{{ MySQL_ResultBind::bindResult() -I- */
void MySQL_ResultBind::bindResult()
{
	CPP_ENTER("MySQL_Prepared_Statement::bindResult");
	for (unsigned int i = 0; i < num_fields; ++i) {
		delete[] (char *) rbind[i].buffer;
	}
	rbind.reset(NULL);
	is_null.reset(NULL);
	err.reset(NULL);
	len.reset(NULL);

	num_fields = capi->mysql_stmt_field_count(stmt);
	if (!num_fields) {
		return;
	}

	rbind.reset(new MYSQL_BIND[num_fields]);
	memset(rbind.get(), 0, sizeof(MYSQL_BIND) * num_fields);

	is_null.reset(new my_bool[num_fields]);
	memset(is_null.get(), 0, sizeof(my_bool) * num_fields);

	err.reset(new my_bool[num_fields]);
	memset(err.get(), 0, sizeof(my_bool) * num_fields);

	len.reset(new unsigned long[num_fields]);
	memset(len.get(), 0, sizeof(unsigned long) * num_fields);


	MySQL_ResultsetData resultMeta(capi->mysql_stmt_result_metadata(stmt), capi, logger);

	for (unsigned int i = 0; i < num_fields; ++i) {
		MYSQL_FIELD * field = resultMeta.fetch_field();

		struct st_buffer_size_type p = allocate_buffer_for_field(field);
		rbind[i].buffer_type	= p.type;
		rbind[i].buffer		= p.buffer;
		rbind[i].buffer_length= static_cast<unsigned long>(p.size);
		rbind[i].length		= &len[i];
		rbind[i].is_null	= &is_null[i];
		rbind[i].error		= &err[i];
		rbind[i].is_unsigned = field->flags & UNSIGNED_FLAG;
	}
	if (capi->mysql_stmt_bind_result(stmt, rbind.get())) {
		CPP_ERR_FMT("Couldn't bind : %d:(%s) %s", capi->mysql_stmt_errno(stmt), capi->mysql_stmt_sqlstate(stmt), capi->mysql_stmt_error(stmt));
		sql::mysql::util::throwSQLException(*capi.get(), stmt);
	}
}
/* }}} */


} /* namespace mysql */
} /* namespace sql */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

