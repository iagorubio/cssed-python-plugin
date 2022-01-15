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
#include "cssed-pyobject.h"
#include "pycssed-constants-table.h"

// reference count for the interpreter
static guint refcount = 0;
// constants type
staticforward PyTypeObject pycssed_PyCssedConstantsType;

/* Utils for all this module*/
static PyObject *cssedpy_get_module_object(char *name) {
	PyObject* pycssed_module, *pycssed_dictionarie;
	pycssed_module = PyImport_AddModule("cssed");
	pycssed_dictionarie = PyModule_GetDict(pycssed_module);
	return PyDict_GetItemString(pycssed_dictionarie, name);
}

static PyObject *cssedpy_get_plugin() {
	return cssedpy_get_module_object("plugin");
}

//
// Constants Object: stores all defined values returned by cssed plugin functions
//

//Object initialuzation 
typedef struct {
    PyObject_HEAD
} pycssed_PyCssedConstantsObject;

// constructor
static PyObject*
pycssed_get_constants (PyObject* self, PyObject* args)
{
    pycssed_PyCssedConstantsObject* pycssed_cons;

    if (!PyArg_ParseTuple(args,":new_cssed")) 
        return NULL;
	
    pycssed_cons = PyObject_New(pycssed_PyCssedConstantsObject, &pycssed_PyCssedConstantsType);

    return (PyObject*)pycssed_cons;
}

// deallocation 
static void
pycssed_constants_dealloc(PyObject* self)
{
    PyObject_Del(self);
}

// Attribute management
PyObject *
pycssed_constants_getattr (PyObject *self, char *name)
{
	CssedPlugin *plugin;
	PyObject *pyobj;
	gint value;

  	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
 
 	if( (value = pycssed_constans_lookup_table (plugin, name)) >= 0){
		return Py_BuildValue("i", value);
	}else{
		PyErr_Format (PyExc_AttributeError, "Attribute %s does not exists", name);
		return NULL;
	}
}

static PyTypeObject pycssed_PyCssedConstantsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "CssedConstants",
    sizeof(pycssed_PyCssedConstantsObject),
    0,
    pycssed_constants_dealloc, //tp_dealloc
    0,                         //tp_print
    pycssed_constants_getattr, //tp_getattr
    0,                         //tp_setattr
    0,                         //tp_compare
    0,                         //tp_repr
    0,                         //tp_as_number
    0,                         //tp_as_sequence
    0,                         //tp_as_mapping
    0,                         //tp_hash 
};

/*
DL_EXPORT(void)
pycssed_constants(void) 
{
    pycssed_PyCssedConstantsType.ob_type = &PyType_Type;
}
*/


//
//  CSSED PYTHON API, on cssed module - almost matches the plugin's API
//

static PyObject* 
pycssed_error_message(PyObject *self, PyObject *args)
{
	int parsed;
	const char *message;
	const char *title = NULL;

	parsed = PyArg_ParseTuple(args, "s|s", &message, &title);
	if (parsed) {
		cssed_plugin_error_message( (gchar*) (title!=NULL?title:""), (gchar*) message);
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_add_text_to_document (PyObject *self, PyObject *args)
{
	int parsed;
	const char *text;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "s", &text);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_add_text_to_document( plugin, (gchar*) text );
		return Py_BuildValue("");
	}

	return NULL;
}

static PyObject* 
pycssed_get_text (PyObject *self, PyObject *args)
{
	gchar* text;
	CssedPlugin* plugin;
	PyObject* pyobj, *pystring;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	text = cssed_plugin_get_text ( plugin );
	pystring = Py_BuildValue("s", text);
	g_free(text);
	return pystring;
}

static PyObject* 
pycssed_select_text_range (PyObject *self, PyObject *args)
{
	int parsed;
	int start, end;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "ii", &start, &end);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_select_text_range ( plugin, start, end );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_get_selected_text (PyObject *self, PyObject *args)
{
	gchar* text;
	CssedPlugin* plugin;
	PyObject* pyobj, *pystring;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	text = cssed_plugin_get_selected_text ( plugin );
	if( text ){
		pystring = Py_BuildValue("s", text);
		g_free(text);
	}else{
		pystring = Py_BuildValue("");
	}
	return pystring;
}

static PyObject* 
pycssed_undo_document_action (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_undo_document_action( plugin );
	return Py_BuildValue("");
}

static PyObject* 
pycssed_redo_document_action (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_redo_document_action( plugin );
	return Py_BuildValue("");
}

static PyObject* 
pycssed_search_prev (PyObject *self, PyObject *args)
{
	int parsed;
	gchar *text;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "s", &text);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_search_prev ( plugin, text );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_search_next (PyObject *self, PyObject *args)
{
	int parsed;
	gchar *text;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "s", &text);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_search_next ( plugin,text );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_document_can_undo (PyObject *self, PyObject *args)
{
	gboolean canundo;
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	canundo = cssed_plugin_document_can_undo ( plugin );
	return Py_BuildValue("b", canundo);
}

static PyObject* 
pycssed_document_can_redo (PyObject *self, PyObject *args)
{
	gboolean canredo;
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	canredo = cssed_plugin_document_can_undo ( plugin );
	return Py_BuildValue("b", canredo);
}
////////////////////////////////////////////////////////////////////////
static PyObject* 
pycssed_bookmark_next (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_bookmark_next ( plugin );
	return Py_BuildValue("");
}

static PyObject* 
pycssed_bookmark_prev (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_bookmark_prev ( plugin );
	return Py_BuildValue("");
}

static PyObject* 
pycssed_set_bookmark_at_line (PyObject *self, PyObject *args)
{
	int parsed;
	gint line;
	gint set;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "ib", &line, &set);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_bookmark_at_line ( plugin, (gboolean) set, line );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_is_bookmark_set_at_line (PyObject *self, PyObject *args)
{
	int parsed;
	gint line;
	gboolean isset;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		isset = cssed_plugin_is_bookmark_set_at_line ( plugin,  line );
		return Py_BuildValue("b", isset);
	}
	return NULL;
}


static PyObject* 
pycssed_marker_next (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	gboolean marker_exists;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	marker_exists = cssed_plugin_marker_next ( plugin );
	return Py_BuildValue("b", marker_exists);
}

static PyObject* 
pycssed_marker_prev (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	gboolean marker_exists;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	marker_exists = cssed_plugin_marker_prev ( plugin );
	return Py_BuildValue("b", marker_exists);
}

static PyObject* 
pycssed_set_marker_at_line (PyObject *self, PyObject *args)
{
	int parsed;
	gint line;
	gint set;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "ib", &line, &set);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_marker_at_line ( plugin, (gboolean) set, line );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_is_marker_set_at_line (PyObject *self, PyObject *args)
{
	int parsed;
	gint line;
	gboolean isset;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		isset = cssed_plugin_is_marker_set_at_line ( plugin,  line );
		return Py_BuildValue("b", isset);
	}
	return NULL;
}


static PyObject* 
pycssed_output_write (PyObject *self, PyObject *args)
{
	int parsed;
	gchar *markup;
	CssedPlugin* plugin;
	PyObject* pyobj;

	parsed = PyArg_ParseTuple(args, "s", &markup);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_output_write ( plugin,  markup );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_output_clear (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	cssed_plugin_output_clear (plugin);
	return Py_BuildValue("");
}
//
static PyObject* 
pycssed_get_line_from_position(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint line, pos;
	int parsed;

	parsed = PyArg_ParseTuple(args, "i", &pos);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		line = cssed_plugin_get_line_from_position ( plugin,  pos );
		return Py_BuildValue("i", line);
	}
	return NULL;
}

static PyObject* 
pycssed_get_position_from_line(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint line, pos;
	int parsed;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		pos = cssed_plugin_get_position_from_line ( plugin,  line );
		return Py_BuildValue("i", pos);
	}
	return NULL;
}
static PyObject* 
pycssed_get_current_position(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint pos;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	pos = cssed_plugin_get_current_position (plugin);
	return Py_BuildValue("i", pos);
}

static PyObject* 
pycssed_set_current_position(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint pos;

	parsed = PyArg_ParseTuple(args, "i", &pos);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_current_position ( plugin,  pos );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_set_current_line(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_current_line ( plugin, line );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_get_current_line(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint line;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	line = cssed_plugin_get_current_line (plugin);
	return Py_BuildValue("i", line);
}

static PyObject* 
pycssed_get_line_end_from_position(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line, pos;

	parsed = PyArg_ParseTuple(args, "i", &pos);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		line = cssed_plugin_get_line_end_from_position ( plugin,  pos );
		return Py_BuildValue("i", line);
	}
	return NULL;
}

static PyObject* 
pycssed_cut(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	cssed_plugin_cut (plugin);
	return Py_BuildValue("");
}

static PyObject* 
pycssed_copy(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	cssed_plugin_copy (plugin);
	return Py_BuildValue("");
}

static PyObject* 
pycssed_paste(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	cssed_plugin_paste (plugin);
	return Py_BuildValue("");
}

static PyObject* 
pycssed_get_selection_end(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint end;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	end = cssed_plugin_get_selection_end (plugin);
	return Py_BuildValue("i", end);
}

static PyObject* 
pycssed_get_selection_start(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint start;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	start = cssed_plugin_get_selection_start (plugin);
	return Py_BuildValue("i", start);
}

static PyObject* 
pycssed_set_selection_start(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint start;

	parsed = PyArg_ParseTuple(args, "i", &start);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_selection_start ( plugin,  start );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_set_selection_end(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint end;

	parsed = PyArg_ParseTuple(args, "i", &end);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_selection_end ( plugin,  end );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_set_selection_range(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint start, end;

	parsed = PyArg_ParseTuple(args, "ii", &start, &end);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_selection_range ( plugin,  start, end );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_replace_sel(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gchar* text;

	parsed = PyArg_ParseTuple(args, "s", &text);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_replace_sel ( plugin, text );
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_clear_sel (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	cssed_plugin_clear_sel (plugin);
	return Py_BuildValue("");
}

static PyObject* 
pycssed_get_length (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint length;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	length = cssed_plugin_get_length (plugin);
	return Py_BuildValue("i", length);
}
static PyObject* 
pycssed_show_user_list (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gchar* list;

	parsed = PyArg_ParseTuple(args, "s", &list);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_show_user_list ( plugin, list );
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_show_autocompletion_list(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint len;
	gchar* list;

	parsed = PyArg_ParseTuple(args, "is", &len, &list);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_show_autocompletion_list ( plugin, len, list );
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_autocompletion_cancel(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	cssed_plugin_autocompletion_cancel (plugin);
	return Py_BuildValue("");
}
static PyObject* 
pycssed_is_autocompletion_active(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr (pyobj);
	if( cssed_plugin_is_autocompletion_active (plugin) )
		return Py_BuildValue("b", TRUE);
	else
		return Py_BuildValue("b", FALSE);
}
static PyObject* 
pycssed_autocompletion_select(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gchar* select;

	parsed = PyArg_ParseTuple(args, "s", &select);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_autocompletion_select ( plugin, select );
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_get_text_line(PyObject *self, PyObject *args)
{
	CssedPlugin *plugin;
	PyObject *pyobj, *pystring;
	int parsed;
	gint line;
	gchar* text;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		text = cssed_plugin_get_text_line (plugin, line);
		if( text == NULL ){
			return Py_BuildValue("");
		}else{
			pystring = Py_BuildValue("s", text);
			g_free (text);
			return pystring;
		}
	}
	return NULL;
}
static PyObject* 
pycssed_get_line_length(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line, length;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		length = cssed_plugin_get_line_length (plugin, line);
		return Py_BuildValue("i", length);
	}
	return NULL;
}
static PyObject* 
pycssed_get_line_count(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint count;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	count = cssed_plugin_get_line_count (plugin);
	return Py_BuildValue("i", count);
}
static PyObject* 
pycssed_toggle_fold_at_line(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_toggle_fold_at_line (plugin, line);
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_get_line_is_visible(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line;
	gboolean visible;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		visible = cssed_plugin_get_line_is_visible (plugin, line);
		return Py_BuildValue("b", visible);
	}
	return NULL;
}
static PyObject* 
pycssed_ensure_line_is_visible (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_ensure_line_is_visible (plugin, line);
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_get_fold_level(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line, level;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		level = cssed_plugin_get_fold_level (plugin, line);
		return Py_BuildValue("i", level);
	}
	return NULL;
}
static PyObject* 
pycssed_get_fold_depth(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line, depth;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		depth = cssed_plugin_get_fold_depth (plugin, line);
		return Py_BuildValue("i", depth);
	}
	return NULL;
}
static PyObject* 
pycssed_set_folding_margin_visible (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gboolean set;

	parsed = PyArg_ParseTuple(args, "b", &set);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_folding_margin_visible (plugin, set);
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_get_folding_margin_visible(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean visible;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	visible = cssed_plugin_get_folding_margin_visible (plugin);
	return Py_BuildValue("b", visible);
}
static PyObject*  
pycssed_get_folding_enabled(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean enabled;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	enabled = cssed_plugin_get_folding_enabled (plugin);
	return Py_BuildValue("b", enabled);
}
static PyObject* 
pycssed_fold_all(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_fold_all (plugin);
	return Py_BuildValue("");
}
static PyObject* 
pycssed_unfold_all(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_unfold_all (plugin);
	return Py_BuildValue("");
}
static PyObject* 
pycssed_set_arrow_marker_at_line(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint line;

	parsed = PyArg_ParseTuple(args, "i", &line);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_arrow_marker_at_line (plugin, line);
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_clear_arrow_marker(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_clear_arrow_marker (plugin);
	return Py_BuildValue("");
}
static PyObject* 
pycssed_zoom_in(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_zoom_in (plugin);
	return Py_BuildValue("");
}
static PyObject* 
pycssed_zoom_out(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_zoom_out (plugin);
	return Py_BuildValue("");
}
static PyObject* 
pycssed_zoom_off(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_zoom_off (plugin);
	return Py_BuildValue("");
}
static PyObject* 
pycssed_get_modified(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean modified;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	modified = cssed_plugin_get_modified (plugin);
	return Py_BuildValue("b", modified);
}

static PyObject* 
pycssed_open_file(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gchar* file;

	parsed = PyArg_ParseTuple(args, "s", &file);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_open_file (plugin, file);
		return Py_BuildValue("");
	}
	return NULL;
}

static PyObject* 
pycssed_is_file_opened(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gchar* file;
	gboolean opened;

	parsed = PyArg_ParseTuple(args, "s", &file);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		opened = cssed_plugin_is_file_opened (plugin, file);
		return Py_BuildValue("b", opened);
	}
	return NULL;
}
static PyObject* 
pycssed_save_document(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean saved;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	saved = cssed_plugin_save_document (plugin);
	return Py_BuildValue("b", saved);
}
static PyObject* 
pycssed_save_document_as(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gchar* file;
	gboolean saved;

	parsed = PyArg_ParseTuple(args, "s", &file);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		saved = cssed_plugin_save_document_as (plugin, file);
		return Py_BuildValue("b", saved);
	}
	return NULL;
}
static PyObject* 
pycssed_get_document_index(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint index;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	index = cssed_plugin_get_document_index (plugin);
	return Py_BuildValue("i", index);
}
static PyObject* 
pycssed_set_document_by_index (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint index;


	parsed = PyArg_ParseTuple(args, "i", &index);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_document_by_index (plugin, index);
		return Py_BuildValue("");
	}
	return NULL;
}
static PyObject* 
pycssed_get_num_docs(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint ndocs;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	ndocs = cssed_plugin_get_num_docs (plugin);
	return Py_BuildValue("i", ndocs);
}
static PyObject* 
pycssed_new_document(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean opened;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	opened = cssed_plugin_new_document (plugin);
	return Py_BuildValue("b", opened);
}
static PyObject* 
pycssed_get_filename(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj, *pystr;
	gchar* filename;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	filename = cssed_plugin_document_get_filename (plugin);
	if( filename != NULL ){
		pystr = Py_BuildValue("s", filename);
		g_free (filename);
		return pystr;
	}else{
		return Py_BuildValue(""); // Py_NONE
	}
}
static PyObject* 
pycssed_get_style_at(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int parsed;
	gint pos, style;

	parsed = PyArg_ParseTuple(args, "i", &pos);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		style = cssed_plugin_get_style_at (plugin, pos);
		return Py_BuildValue("i", style);
	}
	return NULL;
}

/* only python API methods - not in the plugin's API */
static PyObject* 
pycssed_get_global_python_directory(PyObject *self, PyObject *args)
{
	gchar* globaldir;
	PyObject* pystr;	


#ifdef WIN32
	globaldir =  g_win32_get_package_installation_subdirectory
                                            (NULL,
                                             NULL,
											 GLOBAL_SCRIPTS_DIR);
#else
	globaldir = g_strdup_printf ("%s%s", PACKAGE_LIB_DIR, GLOBAL_SCRIPTS_DIR);
#endif
	pystr = Py_BuildValue ("s", globaldir);
	g_free (globaldir);
	return pystr;
}

static PyObject* 
pycssed_get_user_python_directory(PyObject *self, PyObject *args)
{
	gchar* localdir;
	PyObject* pystr;	

	localdir = g_strdup_printf ("%s%s", g_get_home_dir(), USER_SCRIPTS_DIR);
	pystr = Py_BuildValue ("s", localdir);
	g_free (localdir);
	return pystr;
}
// those are included in the plugin API also
static PyObject* 
pycssed_prompt_for_file_to_open(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	gchar *title, *basename = NULL;
	gchar *filename;
	PyObject* pyobj, *pystr;
	int parsed;

	parsed = PyArg_ParseTuple(args, "s|s", &title, &basename);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		filename = cssed_plugin_prompt_for_file_to_open(plugin, title, basename);
		if( filename ){
			pystr = Py_BuildValue("s", filename);
			g_free (filename);
			return pystr;
		}else{
			return Py_BuildValue(""); // None
		}		
	}
	return NULL;	
}

static PyObject* 
pycssed_prompt_for_file_to_save(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	gchar *title, *basename = NULL;
	gchar *filename;
	PyObject* pyobj, *pystr;
	int parsed;

	parsed = PyArg_ParseTuple(args, "s|s", &title, &basename);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		filename = cssed_plugin_prompt_for_file_to_save(plugin, title, basename);
		if( filename ){
			pystr = Py_BuildValue("s", filename);
			g_free (filename);
			return pystr;
		}else{
			return Py_BuildValue(""); // None
		}		
	}
	return NULL;
}
static PyObject* 
pycssed_prompt_for_directory_to_open(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	gchar *title, *basename = NULL;
	gchar *filename;
	PyObject* pyobj, *pystr;
	int parsed;

	parsed = PyArg_ParseTuple(args, "s|s", &title, &basename);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		filename = cssed_plugin_prompt_for_directory_to_open (plugin, title, basename);
		if( filename ){
			pystr = Py_BuildValue("s", filename);
			g_free (filename);
			return pystr;
		}else{
			return Py_BuildValue(""); // None
		}		
	}
	return NULL;
}
static PyObject* 
pycssed_prompt_for_directory_to_create(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	gchar *title, *basename = NULL;
	gchar *filename;
	PyObject* pyobj, *pystr;
	int parsed;

	parsed = PyArg_ParseTuple(args, "s|s", &title, &basename);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		filename = cssed_plugin_prompt_for_directory_to_create (plugin, title, basename);
		if( filename ){
			pystr = Py_BuildValue("s", filename);
			g_free (filename);
			return pystr;
		}else{
			return Py_BuildValue(""); // None
		}		
	}
	return NULL;
}

/*  Interpreter reference counting to hold the interpreter scope.
	The interpreter will be finalized only if reference counting reaches zero
	before to parse a python script. Use with caution or don't use it at all,
	but it will mean no multi-threading nor pygtk */
static PyObject* 
pycssed_interpreter_ref(PyObject *self, PyObject *args)
{
	++refcount;
	return Py_BuildValue("i", refcount);
}
static PyObject* 
pycssed_interpreter_decref(PyObject *self, PyObject *args)
{
	if( refcount > 0 ) --refcount;
	return Py_BuildValue("i", refcount);
}
static PyObject* 
pycssed_interpreter_refcount(PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", refcount);
}
static PyObject* 
pycssed_interpreter_init_script(PyObject *self, PyObject *args)
{
#ifdef WITH_INIT_SCRIPT
	gchar *initscript;
	PyObject *pystr;

	initscript = g_strdup_printf("%s%s%s", g_get_home_dir(), USER_SCRIPTS_DIR, USER_INIT_SCRIPT);
	pystr = Py_BuildValue("s", initscript);
	g_free(initscript);
	return pystr;
#else
	return Py_BuildValue("");// None
#endif
}

static PyObject* 
pycssed_show_calltip(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint start;
	gchar* definition;
	int parsed;

	parsed = PyArg_ParseTuple(args, "is", &start, &definition);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_show_calltip (plugin, start, definition);
	}
	return Py_BuildValue("");// None
}

static PyObject* 
pycssed_cancel_calltip(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_cancel_calltip (plugin);
	return Py_BuildValue("");// None
}


static PyObject* 
pycssed_is_calltip_active(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean active;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	active = cssed_plugin_calltipactive (plugin);
	return Py_BuildValue("b", active) ;
}

static PyObject* 
pycssed_calltip_get_pos_start(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint pos;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	pos = cssed_plugin_calltiposstart (plugin);
	return Py_BuildValue("i", pos) ;
}

static PyObject* 
pycssed_document_grab_focus(PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;

	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	cssed_plugin_document_grab_focus (plugin);
	return Py_BuildValue("");
}


static PyObject* 
pycssed_get_filetype_id (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int id;
	
	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	id = cssed_plugin_get_filetype_id (plugin);
	
	return Py_BuildValue("i", id);
}

static PyObject* 
pycssed_set_filetype_by_id (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint id;
	int parsed;

	parsed = PyArg_ParseTuple(args, "i", &id);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_filetype_by_id (plugin, id);
	}
	return Py_BuildValue("");// None
}

static PyObject* 
pycssed_get_eol_mode (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int eol;
	
	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	eol = cssed_plugin_get_eol_mode (plugin);
	
	return Py_BuildValue("i", eol);
}

static PyObject* 
pycssed_set_eol_mode (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint eol;
	int parsed;

	parsed = PyArg_ParseTuple(args, "i", &eol);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_eol_mode (plugin, eol);
	}
	return Py_BuildValue("");// None
}

static PyObject* 
pycssed_convert_eols (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint eol;
	int parsed;

	parsed = PyArg_ParseTuple(args, "i", &eol);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_convert_eols (plugin, eol);
	}
	return Py_BuildValue("");// None
}

static PyObject* 	
pycssed_get_codepage (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	int codepage;
	
	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	codepage = cssed_plugin_get_codepage (plugin);
	
	return Py_BuildValue("i", codepage);
}

static PyObject* 
pycssed_set_codepage (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gint codepage;
	int parsed;

	parsed = PyArg_ParseTuple(args, "i", &codepage);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_codepage (plugin, codepage);
	}
	return Py_BuildValue("");// None
}

static PyObject* 
pycssed_get_line_endings_visible (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean visible;
	
	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	visible = cssed_plugin_get_line_endings_visible (plugin);
	
	return Py_BuildValue("b", visible);
}

static PyObject* 
pycssed_set_line_endings_visible (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean visible;
	int parsed;
	
	parsed = PyArg_ParseTuple(args, "b", &visible);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_line_endings_visible (plugin, visible);
	}
	return Py_BuildValue("");// None
}

static PyObject* 
pycssed_get_lines_wrapped (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean wrapped;
	
	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	wrapped = cssed_plugin_get_lines_wrapped (plugin);
	
	return Py_BuildValue("b", wrapped);
}

static PyObject* 
pycssed_set_lines_wrapped (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean wrapped;
	int parsed;
	
	parsed = PyArg_ParseTuple(args, "b", &wrapped);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_lines_wrapped (plugin, wrapped);
	}
	return Py_BuildValue("");// None
}


static PyObject* 
pycssed_get_white_spaces_visible (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean visible;
	
	pyobj = cssedpy_get_plugin();
	plugin = PyCObject_AsVoidPtr(pyobj);
	visible = cssed_plugin_get_white_spaces_visible (plugin);
	
	return Py_BuildValue("b", visible);
}

static PyObject* 
pycssed_set_white_spaces_visible (PyObject *self, PyObject *args)
{
	CssedPlugin* plugin;
	PyObject* pyobj;
	gboolean visible;
	int parsed;
	
	parsed = PyArg_ParseTuple(args, "b", &visible);
	if (parsed) {
		pyobj = cssedpy_get_plugin();
		plugin = PyCObject_AsVoidPtr(pyobj);
		cssed_plugin_set_white_spaces_visible (plugin, visible);
	}
	return Py_BuildValue("");// None
}


/* method table */
static PyMethodDef pycssed_methods[] = {
	{"error_message", pycssed_error_message, METH_VARARGS, "shows an error messge to the user"},
	{"add_text", pycssed_add_text_to_document, METH_VARARGS, "adds text to the current document"},
	{"get_text", pycssed_get_text, METH_VARARGS, "get all the text on current document"},
	{"select_text_range", pycssed_select_text_range, METH_VARARGS, "selects the text between positions start and end"},
	{"get_selected_text", pycssed_get_selected_text, METH_VARARGS, "gets the text selected"},
	{"undo", pycssed_undo_document_action, METH_VARARGS, "undo the last action in the undo queue"},
	{"redo", pycssed_redo_document_action, METH_VARARGS, "redo the last action in the redo queue"},
	{"can_undo", pycssed_document_can_undo, METH_VARARGS, "returns if there're actions in the undo queue"},
	{"can_redo", pycssed_document_can_redo, METH_VARARGS, "returns if there're actions in the undo queue"},
	{"search_next", pycssed_search_next, METH_VARARGS, "searchs a string forward in current document"},
	{"search_prev", pycssed_search_prev, METH_VARARGS, "searchs a string backward in current document"},

	{"bookmark_next", pycssed_bookmark_next, METH_VARARGS, "goes to next bookmark or shows an error message if there're no more bookmarks"},
	{"bookmark_prev", pycssed_bookmark_prev, METH_VARARGS, "goes to previous bookmark or shows an error message if there're no previous bookmarks"},
	{"set_bookmark_at_line", pycssed_set_bookmark_at_line, METH_VARARGS, "sets or unsets a bookmark at a given line number"},
	{"is_bookmark_set_at_line", pycssed_is_bookmark_set_at_line, METH_VARARGS, "returns whether a bookmark is set or not at a given line "},

	{"marker_next", pycssed_marker_next, METH_VARARGS, "goes to next marker or shows an error message if there're no more marker"},
	{"marker_prev", pycssed_marker_prev, METH_VARARGS, "goes to previous marker or shows an error message if there're no previous marker"},
	{"set_marker_at_line", pycssed_set_marker_at_line, METH_VARARGS, "sets or unsets a marker at a given line number"},
	{"is_marker_set_at_line", pycssed_is_marker_set_at_line, METH_VARARGS, "returns whether a marker is set or not at a given line"},

	{"output_write", pycssed_output_write, METH_VARARGS, "writes the given message on output window"},
	{"output_clear", pycssed_output_clear, METH_VARARGS, "erases all contents on output window"},

	{"get_line_from_position", pycssed_get_line_from_position, METH_VARARGS, "returns the line number at a given position"},
	{"get_position_from_line", pycssed_get_position_from_line, METH_VARARGS, "returns the character position at given line"},
	{"get_current_position", pycssed_get_current_position, METH_VARARGS, "returns the caret character position"},
	{"set_current_position", pycssed_set_current_position, METH_VARARGS, "sets the current caret position"},
	{"set_current_line", pycssed_set_current_line, METH_VARARGS, "moves the caret to the start of the given line"},
	{"get_current_line", pycssed_get_current_line, METH_VARARGS, "returns the line number where the caret is placed"},
	{"get_line_end_from_position", pycssed_get_line_end_from_position, METH_VARARGS, "returns the end of a line from a given position"},

	{"cut", pycssed_cut, METH_VARARGS, "performs the cut action on current selection if any"},
	{"copy", pycssed_copy, METH_VARARGS, "performs the copy action on current selection if any"},
	{"paste", pycssed_paste, METH_VARARGS, "pastes the contents of the clipboard at caret position"},

	{"get_selection_end", pycssed_get_selection_end, METH_VARARGS, "returns the character position at the end of current selection (if any)"},
	{"get_selection_start", pycssed_get_selection_start, METH_VARARGS, "returns the character position at the start of current selection (if any)"},
	{"set_selection_start", pycssed_set_selection_start, METH_VARARGS, "sets the start of current selection at the given character position"},
	{"set_selection_end", pycssed_set_selection_end, METH_VARARGS, "sets the end of current selection at the given character position"},
	{"set_selection_range", pycssed_set_selection_range, METH_VARARGS, "sets the start and end of current selection at the given character positions"},
	{"replace_sel", pycssed_replace_sel, METH_VARARGS, "replaces the current selection with the given text"},
	{"clear_sel", pycssed_clear_sel, METH_VARARGS, "erases all text selected"},	

	{"get_length", pycssed_get_length, METH_VARARGS, "returns the character length of current document"},

	{"show_user_list", pycssed_show_user_list, METH_VARARGS, "shows a set of strings to be choosen by the user"},

	{"show_autocompletion_list", pycssed_show_autocompletion_list, METH_VARARGS, "shows a list of strings that may complete the current selected word"},
	{"autocompletion_cancel", pycssed_autocompletion_cancel, METH_VARARGS, "hiddes the user list, or auto completion list if any"},
	{"is_autocompletion_active", pycssed_is_autocompletion_active, METH_VARARGS, "returns a boolean indicating if any user list or auto completion list is being shown"},
	{"autocompletion_select", pycssed_autocompletion_select, METH_VARARGS, "selects an item in an auto completion list"},

	{"get_text_line", pycssed_get_text_line, METH_VARARGS, "gets a line of text"},
	{"get_line_length", pycssed_get_line_length, METH_VARARGS, "gets the character length on a line"},
	{"get_line_count", pycssed_get_line_count, METH_VARARGS, "returns the number of lines in the document"},

	{"toggle_fold_at_line", pycssed_toggle_fold_at_line, METH_VARARGS, ""},
	{"get_line_is_visible", pycssed_get_line_is_visible, METH_VARARGS, ""},
	{"ensure_line_is_visible", pycssed_ensure_line_is_visible, METH_VARARGS, ""},
	{"get_fold_level", pycssed_get_fold_level, METH_VARARGS, ""},
	{"get_fold_depth", pycssed_get_fold_depth, METH_VARARGS, ""},
	{"set_folding_margin_visible", pycssed_set_folding_margin_visible, METH_VARARGS, ""},
	{"get_folding_margin_visible", pycssed_get_folding_margin_visible, METH_VARARGS, ""},
	{"get_folding_enabled", pycssed_get_folding_enabled, METH_VARARGS, ""},
	{"fold_all", pycssed_fold_all, METH_VARARGS, ""},
	{"unfold_all", pycssed_unfold_all, METH_VARARGS, ""},

	{"set_arrow_marker_at_line", pycssed_set_arrow_marker_at_line, METH_VARARGS, ""},
	{"clear_arrow_marker", pycssed_clear_arrow_marker, METH_VARARGS, ""},

	{"zoom_in", pycssed_zoom_in, METH_VARARGS, ""},
	{"zoom_out", pycssed_zoom_out, METH_VARARGS, ""},
	{"zoom_off", pycssed_zoom_off, METH_VARARGS, ""},

	{"get_modified", pycssed_get_modified, METH_VARARGS, ""},

	{"open_file", pycssed_open_file, METH_VARARGS, ""},
	{"is_file_opened", pycssed_is_file_opened, METH_VARARGS, ""},
	{"save_document", pycssed_save_document, METH_VARARGS, ""},
	{"save_document_as", pycssed_save_document_as, METH_VARARGS, ""},
	{"get_document_index", pycssed_get_document_index, METH_VARARGS, ""},
	{"set_document_by_index", pycssed_set_document_by_index, METH_VARARGS, ""},
	{"get_num_docs", pycssed_get_num_docs, METH_VARARGS, ""},
	{"new_document", pycssed_new_document, METH_VARARGS, ""},
	{"get_filename", pycssed_get_filename, METH_VARARGS, ""},

	{"get_style_at", pycssed_get_style_at, METH_VARARGS, ""},

	{"get_global_python_directory", pycssed_get_global_python_directory, METH_VARARGS, ""},
	{"get_user_python_directory", pycssed_get_user_python_directory, METH_VARARGS, ""},

	{"prompt_for_file_to_open", pycssed_prompt_for_file_to_open, METH_VARARGS, ""},
	{"prompt_for_file_to_save", pycssed_prompt_for_file_to_save, METH_VARARGS, ""},
	{"prompt_for_directory_to_open", pycssed_prompt_for_directory_to_open, METH_VARARGS, ""},
	{"prompt_for_directory_to_create", pycssed_prompt_for_directory_to_create, METH_VARARGS, ""},

	{"interpreter_ref", pycssed_interpreter_ref, METH_VARARGS, ""},
	{"interpreter_decref", pycssed_interpreter_decref, METH_VARARGS, ""},
	{"interpreter_refcount", pycssed_interpreter_refcount, METH_VARARGS, ""},
	{"interpreter_init_script", pycssed_interpreter_init_script, METH_VARARGS, "returns the interpreter initiall script file or None if it's not supported"},

	{"show_calltip", pycssed_show_calltip, METH_VARARGS, ""},
	{"cancel_calltip", pycssed_cancel_calltip, METH_VARARGS, ""},
	{"is_calltip_active", pycssed_is_calltip_active, METH_VARARGS, ""},
	{"calltip_get_pos_start", pycssed_calltip_get_pos_start, METH_VARARGS, "returns the start document position where the calltip is if any"},
	
	{"document_grab_focus", pycssed_document_grab_focus, METH_VARARGS, "makes the editor control to grab the focus"},
	
	{"get_filetype_id", pycssed_get_filetype_id, METH_VARARGS, ""},		
	{"set_filetype_by_id", pycssed_set_filetype_by_id, METH_VARARGS, ""},	
	{"get_eol_mode", pycssed_get_eol_mode, METH_VARARGS, ""},	
	{"set_eol_mode", pycssed_set_eol_mode, METH_VARARGS, ""},	
	{"get_codepage", pycssed_get_codepage, METH_VARARGS, ""},	
	{"set_codepage", pycssed_set_codepage, METH_VARARGS, ""},	
	{"get_line_endings_visible", pycssed_get_line_endings_visible, METH_VARARGS, ""},	
	{"set_line_endings_visible", pycssed_set_line_endings_visible, METH_VARARGS, ""},
	{"get_lines_wrapped", pycssed_get_lines_wrapped, METH_VARARGS, ""},	
	{"set_lines_wrapped", pycssed_set_lines_wrapped, METH_VARARGS, ""},	
	{"get_eol_mode", pycssed_get_eol_mode, METH_VARARGS, ""},	
	{"set_eol_mode", pycssed_set_eol_mode, METH_VARARGS, ""},	
	{"convert_eols", pycssed_convert_eols, METH_VARARGS, ""},
	{"get_white_spaces_visible", pycssed_get_white_spaces_visible, METH_VARARGS, ""},
	{"set_white_spaces_visible", pycssed_set_white_spaces_visible, METH_VARARGS, ""},	

	// returns and instance of a "constants" object
	{"constants", pycssed_get_constants, METH_VARARGS, "reurns an instance of a PyCsssedConstants object with some defined values returned by cssed's plugin functions"},
	
	{NULL, NULL, 0, NULL}
};

/* Highly inspired in seamonkey's Python xpcom
   error handling */
#define TB_GOTO_END(str) {error_message = ""; g_print(str); goto end;}

gchar* pycssed_traceback_as_string(PyObject *exc_tb) //, gint *lineno)
{
		gchar *tmp_result = NULL;
        gchar *result = NULL;
        gchar *error_message = NULL;
        PyObject *stringIO_module = NULL;
        PyObject *traceback_module = NULL;
        PyObject *stringIO_func = NULL;
        PyObject *stringIO_object = NULL;
        PyObject *printtb_func = NULL;
        PyObject *tblineno_func = NULL;
        PyObject *traceback_args = NULL;
        PyObject *result_object = NULL;
        //PyObject *lineno_object = NULL;

        stringIO_module = PyImport_ImportModule("cStringIO");
        if (stringIO_module==NULL)
                TB_GOTO_END("unable to import needed library cStringIO\n");
        stringIO_func = PyObject_GetAttrString(stringIO_module, "StringIO");
        if (stringIO_func==NULL)
                TB_GOTO_END("unable to import needed method cStringIO.StringIO\n");
        stringIO_object = PyObject_CallObject(stringIO_func, NULL);
        if (stringIO_object==NULL)
                TB_GOTO_END("unable to run needed method cStringIO.StringIO\n");

        traceback_module = PyImport_ImportModule("traceback");
        if (traceback_module==NULL)
                TB_GOTO_END("unable to import needed library traceback\n");
        printtb_func = PyObject_GetAttrString(traceback_module, "print_tb");
        if (printtb_func==NULL)
				TB_GOTO_END("unable to import method traceback.print_tb\n");
        traceback_args = Py_BuildValue("OOO", 
                        exc_tb  ? exc_tb  : Py_None,
                        Py_None, 
                        stringIO_object);
        if (traceback_args==NULL) 
                TB_GOTO_END("unable to build object traceback_args\n");
        result_object = PyObject_CallObject(printtb_func, traceback_args);
        if (result_object==NULL) 
                TB_GOTO_END("unable to run needed method traceback.print_tb()\n");

        Py_DECREF(stringIO_func);
        stringIO_func = PyObject_GetAttrString(stringIO_object, "getvalue");
        if (stringIO_func==NULL)
                TB_GOTO_END("unable to import needed method getvalue\n");
        Py_DECREF(result_object);
        result_object = PyObject_CallObject(stringIO_func, NULL);
        if (result_object==NULL) 
                TB_GOTO_END("unable to run needed method getvalue().\n");
        if (!PyString_Check(result_object))
                TB_GOTO_END("unable to get a string from method getvalue()\n");


        tmp_result = PyString_AsString(result_object);
        result = g_strdup (tmp_result);
/*
		if( lineno ){
			tblineno_func = PyObject_GetAttrString(traceback_module, "tb_lineno");
			if (tblineno_func==NULL)
					TB_GOTO_END("unable to import method traceback.tb_lineno\n");
			lineno_object = PyObject_CallObject(tblineno_func, exc_tb);
			if (lineno_object==NULL)
					TB_GOTO_END("unable to call method traceback.tb_lineno\n");
			*lineno = 
		}
*/
end:
        if (result == NULL && error_message != NULL) 
             result = g_strconcat ("Traceback error:\n", error_message, NULL);
        
        Py_XDECREF(stringIO_module);
        Py_XDECREF(traceback_module);
        Py_XDECREF(stringIO_func);
        Py_XDECREF(stringIO_object);
        Py_XDECREF(printtb_func);
        Py_XDECREF(tblineno_func);
       // Py_XDECREF(tblineno_object);
        Py_XDECREF(traceback_args);
        Py_XDECREF(result_object);
        
        return result;
}
/* seamonkey's inspiration ends */

gchar*
pycssed_format_error (void)
{
	gchar *string, *iter;
	PyObject *errobj = NULL, *errdata = NULL, *errtraceback = NULL, *pystring;

	PyErr_Fetch(&errobj, &errdata, &errtraceback);
	PyErr_NormalizeException(&errobj, &errdata, &errtraceback);

	if( errobj == NULL && errdata == NULL && errtraceback == NULL )
		return NULL;

	pystring = NULL;
	if( errobj != NULL ){
		pystring = PyObject_Str (errobj);
		string = g_strconcat (PyString_AsString (pystring), "\n\n", NULL);
		Py_XDECREF(pystring);
	}

	pystring = NULL;
	if ( errdata != NULL )
	{
		iter = string;
		pystring = PyObject_Str (errdata);
		string = g_strconcat (iter, PyString_AsString(pystring), NULL);
		Py_XDECREF(pystring);
		if( iter ) g_free (iter);
	}else{		
		string = g_strdup ( "unknown exception" );
	}

	if( errtraceback != NULL ){
		iter = string;
		string = pycssed_traceback_as_string (errtraceback);
		if( string && iter ){
			gchar *tmp = string;
			string = g_strconcat (iter, "\n\n", string, NULL);
			g_free (iter);
			g_free (tmp);
		}else if( iter && !string ){
			string = iter;
		}
	}

	Py_XDECREF(errobj);
	Py_XDECREF(errdata);
	Py_XDECREF(errtraceback);

	return string;
}

void
pycssed_show_error ()
{
	gchar *error_message;
	error_message = pycssed_format_error ();
	if( error_message ){
		cssed_plugin_error_message( _("Unable to execute script"), error_message);
		g_free (error_message);
	}
}
void 
pycssed_run_file_by_callback_data (PythonScriptCallbackData* data)
{
	gchar* filename;
	CssedPlugin* plugin;
	PyObject *pydict;
	PyObject *obj;
	gchar *script;
	gchar **argv;
	
	filename = data->scriptname;
	plugin = data->plugin;

	if( refcount == 0 && !Py_IsInitialized() ) pycssed_initialize(plugin);
	
	argv = g_strsplit (data->scriptname, "#", 1);
	PySys_SetArgv(g_strv_length(argv), argv);
	g_strfreev (argv);	
	
	pydict = PyDict_New();
	PyDict_SetItemString(pydict, "__builtins__", PyEval_GetBuiltins());

	if(g_file_get_contents (filename, &script, NULL, NULL) ){
		if( (obj = PyRun_String (script,
							Py_file_input,
							pydict,
							pydict 
						 )) == NULL	)
		{
			pycssed_show_error ();
		}else{
			Py_XDECREF(obj);
		}
		g_free( script );
	}else{
		g_print("Unable to get contents %s\n", filename);
	}

	Py_XDECREF(pydict);
	if( refcount == 0 ) pycssed_finalize();
}

void 
pycssed_run_buffer (CssedPlugin* plugin, gchar* buffer)
{
	PyObject *pydict, *obj;
	gchar *argv[] = { "" , NULL };
	
	if( refcount == 0 && !Py_IsInitialized() ) pycssed_initialize(plugin);	
	
	PySys_SetArgv(1, argv);	
	pydict = PyDict_New();
	PyDict_SetItemString(pydict, "__builtins__", PyEval_GetBuiltins());

	if( (obj = PyRun_String ( buffer,
								Py_file_input,
								pydict,
								pydict 
							)) == NULL )
	{
		pycssed_show_error ();
	}else{
		Py_XDECREF(obj);
	}

	Py_XDECREF(pydict);
	if( refcount == 0 ) pycssed_finalize();
}


// initialization
void
pycssed_initialize ( CssedPlugin *plugin )
{
	PyObject *pycssed_module, *pyobjplugin = NULL;
	gchar *env;
#ifdef WITH_INIT_SCRIPT
	FILE *f;
	gchar *initscript;
#endif
#ifdef WIN32
	gchar *globaldir;
#endif

	if (!plugin) return;

	pycssed_PyCssedConstantsType.ob_type = &PyType_Type;
	
	Py_Initialize();

	/* set some directories on sys.path */
#ifdef WIN32
	globaldir =  g_win32_get_package_installation_directory  (NULL, NULL);
	env = g_strdup_printf( "import sys\nsys.path += ['%s%s','%s%s']\n" , globaldir , GLOBAL_SCRIPTS_DIR, g_get_home_dir(), USER_SCRIPTS_DIR);
	g_free(globaldir);
#else
	env = g_strdup_printf( "import sys\nsys.path += ['%s%s','%s%s']\n" , PACKAGE_LIB_DIR , GLOBAL_SCRIPTS_DIR, g_get_home_dir(), USER_SCRIPTS_DIR);
#endif
	PyRun_SimpleString(env);
	g_free(env);	

	// load the cssed module
	Py_InitModule3 ("cssed", pycssed_methods, _("this module provides access to cssed's plugable interface through python"));
	pycssed_module = PyImport_AddModule ("cssed");	
	pyobjplugin = PyCObject_FromVoidPtr ((void *)plugin, NULL);
	PyModule_AddObject (pycssed_module, "plugin", pyobjplugin);
	
#ifdef WITH_INIT_SCRIPT
	/* run initial environment file if any */
	initscript = g_strdup_printf("%s%s%s", g_get_home_dir(), USER_SCRIPTS_DIR, USER_INIT_SCRIPT);
	if( g_file_test(initscript, G_FILE_TEST_EXISTS) ){
		f = fopen(initscript, "r");
		if( f ){
			if( PyRun_SimpleFile(f, initscript) == -1 ){
				cssed_plugin_error_message( _("Unable to execute init script"), _("Error executing init script.\n%s."), initscript);
			}
			fclose(f);
		}
	}
	g_free (initscript);
#endif
}

void
pycssed_finalize ()
{
	// FIXME - is needed more cleanup ?
	Py_Finalize ();
}


