/*  python bindings for cssed's plugable interface (c) Iago Rubio 2004-2005
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <Python.h>
#include <gtk/gtk.h>
#include <plugin.h>

#include "pythonscript.h"
#include "pycssed-constants-table.h"

typedef struct _CssedPyConstants {
	gchar* name;
	gint value;
} CssedPyConstants;

static CssedPyConstants constants[] = {
	// cssed errors
	{ "FILE_ERROR_EXIST", CSSED_FILE_ERROR_EXIST },
	{ "FILE_ERROR_ISDIR", CSSED_FILE_ERROR_ISDIR },
	{ "FILE_ERROR_ACCES", CSSED_FILE_ERROR_ACCES },
	{ "FILE_ERROR_NAMETOOLONG", CSSED_FILE_ERROR_NAMETOOLONG },
	{ "FILE_ERROR_NOENT", CSSED_FILE_ERROR_NOENT },
	{ "FILE_ERROR_NOTDIR", CSSED_FILE_ERROR_NOTDIR },
	{ "FILE_ERROR_NXIO", CSSED_FILE_ERROR_NXIO },
	{ "FILE_ERROR_NODEV", CSSED_FILE_ERROR_NODEV },
	{ "FILE_ERROR_ROFS", CSSED_FILE_ERROR_ROFS },
	{ "FILE_ERROR_TXTBSY", CSSED_FILE_ERROR_TXTBSY },
	{ "FILE_ERROR_FAULT", CSSED_FILE_ERROR_FAULT },
	{ "FILE_ERROR_LOOP", CSSED_FILE_ERROR_LOOP },
	{ "FILE_ERROR_NOSPC", CSSED_FILE_ERROR_NOSPC },
	{ "FILE_ERROR_NOMEM", CSSED_FILE_ERROR_NOMEM },
	{ "FILE_ERROR_MFILE", CSSED_FILE_ERROR_MFILE },
	{ "FILE_ERROR_NFILE", CSSED_FILE_ERROR_NFILE },
	{ "FILE_ERROR_BADF", CSSED_FILE_ERROR_BADF },
	{ "FILE_ERROR_INVAL", CSSED_FILE_ERROR_INVAL },
	{ "FILE_ERROR_PIPE", CSSED_FILE_ERROR_PIPE },
	{ "FILE_ERROR_AGAIN", CSSED_FILE_ERROR_AGAIN },
	{ "FILE_ERROR_INTR", CSSED_FILE_ERROR_INTR },
	{ "FILE_ERROR_IO", CSSED_FILE_ERROR_IO },
	{ "FILE_ERROR_PERM", CSSED_FILE_ERROR_PERM },
	{ "FILE_ERROR_FAILED", CSSED_FILE_ERROR_FAILED },
	{ "FILE_ERROR_LAST", CSSED_FILE_ERROR_LAST }, 
	{ "ERROR_FILE_IS_OPENED", CSSED_ERROR_FILE_IS_OPENED }, 
	{ "ERROR_FILE_NOT_REGULAR", CSSED_ERROR_FILE_NOT_REGULAR }, 
	{ "ERROR_DOC_IS_NULL", CSSED_ERROR_DOC_IS_NULL }, 
	// end of line modes
	{ "EOL_MODE_CRLF", EOL_MODE_CRLF },
	{ "EOL_MODE_CR", EOL_MODE_CR },
	{ "EOL_MODE_LF", EOL_MODE_LF },
	// codepage
	{ "CODEPAGE_DEFAULT", CSSED_CODEPAGE_DEFAULT },
	{ "CODEPAGE_UTF8", CSSED_CODEPAGE_UTF8 },
	{ "CODEPAGE_DBCS", CSSED_CODEPAGE_DBCS },
	// file type
	{ "FILETYPE_UNKNOW", CSSED_FILETYPE_UNKNOW },		
	{ "FILETYPE_TEXT", CSSED_FILETYPE_TEXT },		
	{ "FILETYPE_BYNARY", CSSED_FILETYPE_BYNARY },		
	{ "FILETYPE_CSS", CSSED_FILETYPE_CSS },			
	{ "FILETYPE_HTML", CSSED_FILETYPE_HTML },		
	{ "FILETYPE_PHP", CSSED_FILETYPE_PHP },			
	{ "FILETYPE_XML", CSSED_FILETYPE_XML },			
	{ "FILETYPE_SH", CSSED_FILETYPE_SH },			
	{ "FILETYPE_C", CSSED_FILETYPE_C },			
	{ "FILETYPE_APACHE_CONF", CSSED_FILETYPE_APACHE_CONF },	
	{ "FILETYPE_LISP", CSSED_FILETYPE_LISP },		
	{ "FILETYPE_PERL", CSSED_FILETYPE_PERL },		
	{ "FILETYPE_PYTHON", CSSED_FILETYPE_PYTHON },		
	{ "FILETYPE_SQL", CSSED_FILETYPE_SQL },			
	{ "FILETYPE_DIFF", CSSED_FILETYPE_DIFF },		
	{ "FILETYPE_JAVA", CSSED_FILETYPE_JAVA },		
	{ "FILETYPE_JS", CSSED_FILETYPE_JS },			
	{ "FILETYPE_MAKE", CSSED_FILETYPE_MAKE },		
	{ "FILETYPE_APDL", CSSED_FILETYPE_APDL },		
	{ "FILETYPE_AU3", CSSED_FILETYPE_AU3 },
	{ "FILETYPE_AVE", CSSED_FILETYPE_AVE },
	{ "FILETYPE_ADA", CSSED_FILETYPE_ADA },
	{ "FILETYPE_ASM", CSSED_FILETYPE_ASM },
	{ "FILETYPE_ASN1", CSSED_FILETYPE_ASN1 },
	{ "FILETYPE_BAAN", CSSED_FILETYPE_BAAN },
	{ "FILETYPE_BASIC", CSSED_FILETYPE_BASIC },
	{ "FILETYPE_BULLANT", CSSED_FILETYPE_BULLANT },
	{ "FILETYPE_CLW", CSSED_FILETYPE_CLW },
	{ "FILETYPE_CAML", CSSED_FILETYPE_CAML },
	{ "FILETYPE_CRONTAB", CSSED_FILETYPE_CRONTAB },
	{ "FILETYPE_ESCRIPT", CSSED_FILETYPE_ESCRIPT },
	{ "FILETYPE_EIFFEL", CSSED_FILETYPE_EIFFEL },
	{ "FILETYPE_ERLANG", CSSED_FILETYPE_ERLANG },
	{ "FILETYPE_FORTH", CSSED_FILETYPE_FORTH },
	{ "FILETYPE_FORTRAN", CSSED_FILETYPE_FORTRAN },
	{ "FILETYPE_GUI4CLI", CSSED_FILETYPE_GUI4CLI },
	{ "FILETYPE_KIX", CSSED_FILETYPE_KIX },
	{ "FILETYPE_LOUT", CSSED_FILETYPE_LOUT },
	{ "FILETYPE_LUA", CSSED_FILETYPE_LUA },
	{ "FILETYPE_MMIXAL", CSSED_FILETYPE_MMIXAL },
	{ "FILETYPE_MPT", CSSED_FILETYPE_MPT },
	{ "FILETYPE_MSSQL", CSSED_FILETYPE_MSSQL },
	{ "FILETYPE_MATLAB", CSSED_FILETYPE_MATLAB },
	{ "FILETYPE_METAPOST", CSSED_FILETYPE_METAPOST },
	{ "FILETYPE_NSIS", CSSED_FILETYPE_NSIS },
	{ "FILETYPE_PB", CSSED_FILETYPE_PB },
	{ "FILETYPE_POV", CSSED_FILETYPE_POV },
	{ "FILETYPE_PS", CSSED_FILETYPE_PS },
	{ "FILETYPE_PASCAL", CSSED_FILETYPE_PASCAL },
	{ "FILETYPE_RUBY", CSSED_FILETYPE_RUBY },
	{ "FILETYPE_SCRIPTOL", CSSED_FILETYPE_SCRIPTOL },
	{ "FILETYPE_SPECMAN", CSSED_FILETYPE_SPECMAN },
	{ "FILETYPE_TEX", CSSED_FILETYPE_TEX },
	{ "FILETYPE_VB", CSSED_FILETYPE_VB },
	{ "FILETYPE_VHDL", CSSED_FILETYPE_VHDL },
	{ "FILETYPE_VERILOG", CSSED_FILETYPE_VERILOG },
	{ "FILETYPE_YAML", CSSED_FILETYPE_YAML },
	// those are not used by cssed
	{ "FILETYPE_USR", CSSED_FILETYPE_USR },	
	{ "FILETYPE_USR1", CSSED_FILETYPE_USR1 },
	{ "FILETYPE_USR2", CSSED_FILETYPE_USR2 },
	{ "FILETYPE_USR3", CSSED_FILETYPE_USR3 },
	{ "FILETYPE_USR4", CSSED_FILETYPE_USR4 },
	{ "FILETYPE_USR5", CSSED_FILETYPE_USR5 },
	{ "FILETYPE_USR6", CSSED_FILETYPE_USR6 },
	{ "FILETYPE_USR7", CSSED_FILETYPE_USR7 },
	{ "FILETYPE_USR8", CSSED_FILETYPE_USR8 },
	{ "FILETYPE_USR9", CSSED_FILETYPE_USR9 },
	
	{NULL, 0} /* sentinel */
};


void
pycssed_constans_initialize_table (CssedPlugin* plugin)
{
	GHashTable *table;
	CssedPyConstants *iterator;
	PythonPluginData *data;
	
	table = g_hash_table_new  (g_str_hash,  g_str_equal);
	iterator = constants;
	
	while( iterator->name != NULL ){
		g_hash_table_insert (table, iterator->name, GINT_TO_POINTER(iterator->value));
		iterator++;
	}
	
	data = (PythonPluginData*) plugin->user_data;
	data->constants_table = table;
}

void
pycssed_constans_destroy_table (CssedPlugin* plugin)
{
	GHashTable *table;
	PythonPluginData *data;
		
	data = (PythonPluginData*) plugin->user_data;
	table = data->constants_table;	
	
	g_hash_table_destroy (table);
}

gint
pycssed_constans_lookup_table (CssedPlugin* plugin, gchar* key)
{
	GHashTable *table;
	PythonPluginData *data;
	gpointer value;
	
	data = (PythonPluginData*) plugin->user_data;
	table = data->constants_table;
	
	value = g_hash_table_lookup (table, key);
	
	if( value )	return GPOINTER_TO_INT(value);
	else return -1; // no 0< value present so negative return is flagging failure
}

